#include <QCoreApplication>

#include "tcpserver.h"
#include "testsender.h"

#ifdef _MSC_VER
#pragma comment(lib, "WS2_32.lib")
#endif

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	tcpserver *server = new tcpserver;

	TestSender* test = new TestSender;
	test->setFilename("test.bin");
	test->moveToThread(test);
	test->startThread();

	QObject::connect(test, SIGNAL(sendPacket(QByteArray)), server, SIGNAL(sendPacket(QByteArray)), Qt::QueuedConnection);

	server->moveToThread(server);
	server->start();

	test->startPlay();

	return a.exec();
}
