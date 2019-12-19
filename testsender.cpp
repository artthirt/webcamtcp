#include "testsender.h"

#include <QCoreApplication>

#include "common.h"

#define ARRAY_SIZE 1024 * 1024

TestSender::TestSender(QObject *parent) : QThread(parent)
{
    m_sender = nullptr;
	m_timeout = 31;
	m_packetsSended = 0;
}

TestSender::~TestSender()
{
    quit();
	wait();
}

void TestSender::setFilename(const QString &name)
{
    m_fileName = name;

}

QString TestSender::fileName() const
{
	return m_fileName;
}

bool TestSender::isOpenFile()
{
	return m_file.isOpen();
}

float TestSender::progress() const
{
	if(!m_file.isOpen() || m_file.size() == 0)
		return 0;
	float d = (float)m_file.pos()/m_file.size();
	return d;
}

void TestSender::setSender(QObject *sender)
{
    m_sender = sender;
}

void TestSender::startThread()
{
    moveToThread(this);
    start();
}

bool TestSender::startPlay()
{
    if(m_fileName.isEmpty() || !QFile::exists(m_fileName)){
        qDebug("File not exists or not set");
		return false;
    }

    m_mutex.lock();
    if(m_file.isOpen())
        m_file.close();
    m_file.setFileName(m_fileName);
    m_file.open(QIODevice::ReadOnly);
    m_file.seek(0);
    m_mutex.unlock();

	return true;
}

void TestSender::stopPlay()
{
    m_mutex.lock();
    if(m_file.isOpen())
        m_file.close();
    m_mutex.unlock();
}

void TestSender::onTimeout()
{
	if(m_fileName.isEmpty() || !m_file.isOpen()){
        return;
    }

    m_mutex.lock();
    uint size = 0;
    m_file.read((char*)&size, sizeof(size));
    if(size > 99999999){
        m_mutex.lock();
        m_file.close();
        m_mutex.unlock();
        return;
    }
    QByteArray data = m_file.read(size);

//	QFile f("test.h264");
//	f.open(QIODevice::WriteOnly | QIODevice::Append);
//	f.write(data);
//	f.close();

	if(m_file.atEnd()){
		m_file.seek(0);
		//m_file.close();
	}
    m_mutex.unlock();

    if(!data.isEmpty()){
		//printf("packets readed %d        \r", m_packetsSended++);
		if(m_sender){
			QCoreApplication::postEvent(m_sender, new EventTest(data));
		}else
			emit sendPacket(data);
    }else{
        m_mutex.lock();
        m_file.close();
        m_mutex.unlock();
	}
}

void TestSender::run()
{
    m_timer.reset(new QTimer);
    m_timer->moveToThread(this);
    connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->start(m_timeout);

    exec();

    m_timer.reset();
}
