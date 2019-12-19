#include "tcpserver.h"

#include <tcpsocket.h>

tcpserver::tcpserver(QObject *parent) : QThread(parent)
  , m_bindPort(1443)
{
	qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

	m_owner = nullptr;
	m_tcpserver = nullptr;
	connect(this, SIGNAL(changeBindPort()), this, SLOT(onChangeBindPort()), Qt::QueuedConnection);
}

tcpserver::~tcpserver()
{
	quit();
	wait();
}

void tcpserver::setOwner(QObject *owner)
{
	m_owner = owner;
}

void tcpserver::setBindPort(ushort port)
{
	m_bindPort = port;
	if(m_tcpserver && m_tcpserver->isListening()){
		m_tcpserver->close();
	}
	emit onChangeBindPort();
}

void tcpserver::onChangeBindPort()
{
	if(!m_tcpserver)
		return;

	m_tcpserver->listen(QHostAddress::Any, m_bindPort);
}

void tcpserver::onNewConnection()
{
	QTcpSocket *sock = m_tcpserver->nextPendingConnection();

	connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
	connect(sock, SIGNAL(disconnected()), this, SLOT(onDisconnect()), Qt::QueuedConnection);

	Ptcpsocket tcpsock;
	tcpsock.reset(new tcpsocket());
	tcpsock->setSocket(sock);
	tcpsock->setOwner(m_owner);
	connect(this, SIGNAL(sendPacket(QByteArray)), tcpsock.get(), SLOT(sendPacket(QByteArray)), Qt::QueuedConnection);
	m_sockets.push_back( tcpsock );
}

void tcpserver::onDisconnect()
{
	QObject *obj = sender();

	tcpsocket *s = nullptr;

	for(auto ss: m_sockets){
		if(ss.get()->socket() == obj){
			s = ss.get();
			break;
		}
	}

	onCloseSocket(s);
}

void tcpserver::onCloseSocket(tcpsocket *sock)
{
	for(auto it = m_sockets.begin(); it != m_sockets.end(); ++it){
		if(it->get() == sock){
			m_sockets.erase(it);
			break;
		}
	}
}

void tcpserver::onError(QAbstractSocket::SocketError socketError)
{
	onDisconnect();
}

void tcpserver::run()
{
	m_tcpserver = (new QTcpServer());
	m_tcpserver->listen(QHostAddress::Any, m_bindPort);

	connect(m_tcpserver, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

	exec();

	for(auto it: m_sockets){
		it->abort();
	}
	m_tcpserver->close();

	delete m_tcpserver;
}
