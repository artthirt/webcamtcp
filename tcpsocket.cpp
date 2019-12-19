#include "tcpsocket.h"

#include <QDataStream>

#include "common.h"

#include <QCoreApplication>

#ifdef _MSC_VER
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

enum{
	M_BEGIN,
	M_BEGIN_CHECK,
	M_TYPE,
	M_SIZE,
	M_DATA,
	M_END_CHECK,
	M_ERROR
};

bool check_string(char* data, char* string, int len)
{
	int i = 0;
	while(i++ < len && *data++ == *string++){}
	return i == len + 1;
}

tcpsocket::tcpsocket(QObject *parent) : QThread(parent)
{
	m_max_packets = 30;
	m_owner = nullptr;
	m_indexPos = 0;
	m_state = M_BEGIN;
	m_dataPos = 0;
	m_dataSize = 0;
	m_dataBegin = 0;
	m_dataEnd = 0;
	m_ConnectingPort = 1443;
	m_ConnectingHost = QHostAddress("127.0.0.1");
	m_connectedState = false;
	m_tryReconnect = false;
	m_prevError = 0;
	m_isConnected = false;
	m_packetsSended = 0;

	qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

	connect(this, SIGNAL(connectTo()), this, SLOT(onConnectTo()), Qt::QueuedConnection);

//	moveToThread(this);
//	start();
}

tcpsocket::~tcpsocket()
{
	quit();
	wait();
}

void tcpsocket::setSocket(QTcpSocket *sock)
{
	m_socket = sock;

	int buf = 7 * 1024 * 1024;
	setsockopt(m_socket->socketDescriptor(), SOL_SOCKET, SO_RCVBUF, (char*)&buf, sizeof(buf));

	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	connect(m_socket, SIGNAL(connected()), this, SLOT(onConnect()));
	connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
	connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

	printf("new connect %s\r", sock->peerAddress().toString().toLatin1().data());

	m_isConnected = true;
}

void tcpsocket::setOwner(QObject *owner)
{
	m_owner = owner;
}

void tcpsocket::sendPacket(const QByteArray &data)
{
	if(!m_isConnected)
		return;

	m_mutex.lock();
	m_packets.push_back(data);
	m_mutex.unlock();

	while(m_packets.size() > m_max_packets){
		m_mutex.lock();
		m_packets.pop_front();
		m_mutex.unlock();
	}
}

void tcpsocket::connectToHost(const QHostAddress &addr, ushort port)
{
	moveToThread(this);
	start();

	m_socket = new QTcpSocket;
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
	connect(m_socket, SIGNAL(connected()), this, SLOT(onConnect()), Qt::QueuedConnection);
	connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()), Qt::QueuedConnection);
	connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	m_socket->moveToThread(this);

	int buf = 7 * 1024 * 1024;
	setsockopt(m_socket->socketDescriptor(), SOL_SOCKET, SO_RCVBUF, (char*)&buf, sizeof(buf));

	m_ConnectingHost = addr;
	m_ConnectingPort = port;
	m_connectedState = true;
	emit connectTo();
}

void tcpsocket::abort()
{
	if(m_socket){
		m_socket->abort();
		m_socket->close();
	}
}

QTcpSocket *tcpsocket::socket()
{
	return m_socket;
}

void tcpsocket::onReadyRead()
{	
	QByteArray data = m_socket->read(m_socket->size());
	if(!data.isNull())
		parsePacket(data);
}

void tcpsocket::onTimeout()
{
	if(m_isConnected){
		writeNextPacket();
	}

	if(m_tryReconnect){
		m_tryReconnect = false;
		onConnectTo();
	}
}

void tcpsocket::onConnectTo()
{
	if(m_socket)
		m_socket->connectToHost(m_ConnectingHost, m_ConnectingPort);
}

void tcpsocket::onConnect()
{
	printf("new connect %s\n", m_socket->peerAddress().toString().toLatin1().data());
	m_isConnected = true;
}

void tcpsocket::onDisconnect()
{
	m_isConnected = false;
}

void tcpsocket::onError(QAbstractSocket::SocketError error)
{
	m_isConnected = false;
	if(m_connectedState){
		m_tryReconnect = true;
		if(m_prevError == error)
			printf("Error to connect %d           \r", error);
		else
			printf("Error to connect %d           \n", error);
		m_prevError = error;
	}
}

void tcpsocket::run()
{
	m_timer.reset(new QTimer);
	m_timer->moveToThread(this);
	m_timer->start(10);

	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(onTimeout()));

	exec();

	if(m_connectedState && m_socket){
		m_socket->abort();
		delete m_socket;
	}
	m_socket = nullptr;

	m_timer.reset();
}

void tcpsocket::parsePacket(const QByteArray &data)
{
	m_buffer.append(data);

	uint size = m_buffer.size();
	while(m_indexPos < size){
		switch (m_state) {
			case M_BEGIN:
				if(m_buffer[m_indexPos] == tcp_packet::begin_packet[0]){
					m_state = M_BEGIN_CHECK;
				}else
					m_indexPos++;
				break;
			case M_BEGIN_CHECK:{
				if(m_indexPos + tcp_packet::len_begin_packet >= size)
					return;
				if(check_string((char*)&m_buffer.data()[m_indexPos], (char*)tcp_packet::begin_packet, tcp_packet::len_begin_packet)){
					m_state = M_TYPE;
					m_indexPos += tcp_packet::len_begin_packet;
				}else{
					m_state = M_ERROR;
					m_indexPos++;
				}
				break;
			}
			case M_TYPE:
				m_type = m_buffer[m_indexPos];
				if(m_type == 0 || m_type >= tcp_packet::type_end){
					m_state = M_ERROR;
				}else{
					m_state = M_SIZE;
					m_indexPos++;
				}
				break;
			case M_SIZE:{
				if(m_indexPos + sizeof(uint) >= size){
					return;
				}
				m_dataSize = tcp_packet::getUint(&m_buffer.data()[m_indexPos]);
				if(size > 999999999){
					m_state = M_ERROR;
					break;
				}
				m_state = M_DATA;
				m_indexPos += sizeof(uint);
				m_dataPos = 0;
				m_dataBegin = m_indexPos;
				break;
			}
			case M_DATA:{
				if(m_indexPos + m_dataSize >= size){
					return;
				}
				m_dataEnd = m_indexPos + m_dataSize;
				m_state = M_END_CHECK;
				m_indexPos += m_dataSize;
				break;
			}
			case M_END_CHECK:{
				if(m_indexPos + tcp_packet::len_end_packet >= size){
					return;
				}
				if(check_string((char*)&m_buffer.data()[m_indexPos], (char*)tcp_packet::end_packet, tcp_packet::len_end_packet)){
					m_packet.resize(m_dataSize);
					std::copy(&m_buffer.data()[m_dataBegin], &m_buffer.data()[m_dataBegin] + m_dataSize, m_packet.data());
					m_buffer.remove(0, m_indexPos);
					m_indexPos = 0;
					m_state = M_BEGIN;
					sendPacketToOwner();
					return;
				}else{
					m_indexPos = m_dataBegin;
					m_state = M_ERROR;
				}
				break;
			}
			case M_ERROR:
				m_state = M_BEGIN;
				m_buffer.remove(0, m_indexPos);
				m_indexPos = 0;
				return;
		}
	}

}

void tcpsocket::writeNextPacket()
{
	if(m_packets.isEmpty() || !m_socket || !m_socket->isOpen() || !m_isConnected){
		return;
	}

	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);

	QByteArray pkt = m_packets.front();
	uint size = pkt.size();

	tcp_packet::UInt ui = tcp_packet::getUint(size);

	stream.writeRawData(tcp_packet::begin_packet, tcp_packet::len_begin_packet);
	stream.writeRawData((char*)&tcp_packet::type_video, 1);
	stream.writeRawData((char*)ui.uc, 4);
	stream.writeRawData(pkt.data(), pkt.size());
	stream.writeRawData(tcp_packet::end_packet, tcp_packet::len_end_packet);

	{
		m_socket->write(data);
		m_socket->waitForBytesWritten();
	}

	m_mutex.lock();
	m_packets.pop_front();
	m_mutex.unlock();

	printf("packets sender %d            \r", m_packetsSended++);
}

void tcpsocket::sendPacketToOwner()
{
	if(m_owner){
		QCoreApplication::postEvent(m_owner, new EventTest(m_packet, m_type));
	}
}
