#ifndef TESTSENDER_H
#define TESTSENDER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QFile>

#include <memory>
#include <mutex>

class TestSender : public QThread
{
    Q_OBJECT
public:
    explicit TestSender(QObject *parent = nullptr);
    ~TestSender();

    void setFilename(const QString& name);
    QString fileName() const;

	bool isOpenFile();
	float progress() const;

    void setSender(QObject* sender);

    void startThread();

	bool startPlay();
    void stopPlay();

signals:
	void sendPacket(QByteArray);

public slots:
    void onTimeout();

protected:
    virtual void run();

private:
    QFile m_file;
    QString m_fileName;
    int m_timeout;
    std::mutex m_mutex;

    QObject *m_sender;

    std::unique_ptr<QTimer> m_timer;

	uint m_packetsSended;
};

#endif // TESTSENDER_H
