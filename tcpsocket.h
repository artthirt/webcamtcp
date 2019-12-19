#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include <memory>
#include <mutex>

#include "common.h"

class tcpsocket : public QThread
{
	Q_OBJECT
public:
	explicit tcpsocket(QObject *parent = nullptr);
	~tcpsocket();

	void setSocket(QTcpSocket* sock);
	void setOwner(QObject* owner);

	void connectToHost(const QHostAddress& addr, ushort port);

	void abort();

	QTcpSocket *socket();

signals:
	void connectTo();

public slots:
	void onReadyRead();
	void onTimeout();
	void onConnectTo();
	void onConnect();
	void onDisconnect();
	void onError(QAbstractSocket::SocketError error);

	void sendPacket(const QByteArray& data);

protected:
	virtual void run();

private:
	QTcpSocket* m_socket;
	QObject *m_owner;
	QByteArray m_buffer;
	QByteArray m_packet;
	uint m_indexPos;
	uint m_type;
	uint m_dataSize;
	uint m_dataBegin;
	uint m_dataEnd;
	uint m_dataPos;
	uint m_state;
	uint m_packetsSended;

	std::shared_ptr<QTimer> m_timer;

	int m_prevError;
	uint m_max_packets;
	QList< QByteArray > m_packets;
	std::mutex m_mutex;
	QHostAddress m_ConnectingHost;
	ushort m_ConnectingPort;
	bool m_connectedState;
	bool m_tryReconnect;
	bool m_isConnected;

	void parsePacket(const QByteArray& data);
	void writeNextPacket();

	void sendPacketToOwner();
};

#endif // TCPSOCKET_H
