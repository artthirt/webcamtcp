#include "camerastream.h"

#include <QList>
#include <QCameraInfo>

CameraStream::CameraStream(QObject *parent) : QThread(parent)
{
	m_camera = nullptr;
	m_videoSurface = nullptr;
}

CameraStream::~CameraStream()
{
	if(m_camera)
		delete m_camera;

	if(m_videoSurface)
		delete m_videoSurface;
}

void CameraStream::initCamera()
{
	QList<QCameraInfo> cams = QCameraInfo::availableCameras();
	if(cams.empty()){
		printf("camera not found\n");
		return;
	}

	m_videoSurface = new VideoSurface;
	connect(m_videoSurface, SIGNAL(sendImage(QImage)), this, SLOT(onSendImage()), Qt::QueuedConnection);

	m_camera = new QCamera(cams[0]);
	m_camera->setCaptureMode(QCamera::CaptureVideo);

	m_camera->setViewfinder(m_videoSurface);

	m_camera->start();
}

void CameraStream::imageCaptured(int id, const QImage &preview)
{

}

void CameraStream::onSendImage(const QImage &image)
{

}
