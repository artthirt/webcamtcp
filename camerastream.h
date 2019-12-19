#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <QObject>
#include <QThread>
#include <QCamera>
#include "videosurface.h"

class CameraStream : public QThread
{
	Q_OBJECT
public:
	explicit CameraStream(QObject *parent = nullptr);
	~CameraStream();

	void initCamera();

signals:

public slots:
	void imageCaptured(int id, const QImage &preview);
	void onSendImage(const QImage& image);

private:
	QCamera *m_camera;
	VideoSurface *m_videoSurface;
};

#endif // CAMERASTREAM_H
