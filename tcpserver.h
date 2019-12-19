#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

#include <memory>

class tcpsocket;

typedef std::shared_ptr< tcpsocket > Ptcpsocket;

class tcpserver : public QThread
{
	Q_OBJECT
public:
	explicit tcpserver(QObject *parent = nullptr);
	~tcpserver();

	void setOwner(QObject* owner);

	void setBindPort(ushort port);

signals:
	void changeBindPort();
	void sendPacket(const QByteArray& data);

public slots:
	void onChangeBindPort();
	void onNewConnection();
	void onDisconnect();
	void onCloseSocket(tcpsocket*);
	void onError(QAbstractSocket::SocketError socketError);


protected:
	virtual void run();

private:
	QTcpServer *m_tcpserver;
	ushort m_bindPort;
	QObject* m_owner;

	QList<Ptcpsocket> m_sockets;

};

#endif // TCPSERVER_H
