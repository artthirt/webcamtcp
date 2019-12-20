#include <QCoreApplication>

#include "tcpserver.h"
#include "testsender.h"
#include "camerastream.h"

#include <memory>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#ifdef _MSC_VER
#pragma comment(lib, "WS2_32.lib")
#endif

int main(int argc, char *argv[])
{
	av_register_all();
	avcodec_register_all();
    av_log_set_level(AV_LOG_TRACE);

	QCoreApplication a(argc, argv);

	std::unique_ptr<tcpserver> server;
	server.reset(new tcpserver);

//	TestSender* test = new TestSender;
//	test->setFilename("test.bin");
//	test->moveToThread(test);
//	test->startThread();

//	QObject::connect(test, SIGNAL(sendPacket(QByteArray)), server, SIGNAL(sendPacket(QByteArray)), Qt::QueuedConnection);

	std::unique_ptr<CameraStream> camerastream;
	camerastream.reset(new CameraStream);
	camerastream->moveToThread(camerastream.get());
	camerastream->start();

	QObject::connect(camerastream.get(), SIGNAL(sendPacket(QByteArray)), server.get(), SIGNAL(sendPacket(QByteArray)), Qt::QueuedConnection);

	server->moveToThread(server.get());
	server->start();

//	test->startPlay();

	return a.exec();
}
