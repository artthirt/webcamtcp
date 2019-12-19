#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <QObject>
#include <QThread>
#include <QCamera>
#include <QTimer>
#include "videosurface.h"

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class CameraStream : public QThread
{
	Q_OBJECT
public:
	explicit CameraStream(QObject *parent = nullptr);
	~CameraStream();

	bool initCamera();

signals:
	void sendPacket(QByteArray);

public slots:
	void imageCaptured(int id, const QImage &preview);
	void onSendImage(const QImage& image);
	void onTimeout();

	void stateChanged(QCamera::State state);

protected:
	virtual void run();

private:
	QCamera *m_camera;
	std::shared_ptr<QTimer> m_timer;
	VideoSurface *m_videoSurface;
	uint m_numImage;
	QSize m_imageSize;

	AVCodecContext *m_fmt = nullptr;
	AVCodec *m_codec = nullptr;

	bool m_isInitAV;

	void initContext(int width, int height);
};

#endif // CAMERASTREAM_H
