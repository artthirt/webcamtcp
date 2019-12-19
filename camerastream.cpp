#include "camerastream.h"

#include <QList>
#include <QCameraInfo>

CameraStream::CameraStream(QObject *parent) : QThread(parent)
{
	m_camera = nullptr;
	m_videoSurface = nullptr;
	m_numImage = 0;
}

CameraStream::~CameraStream()
{
	quit();
	wait();
}

bool CameraStream::initCamera()
{
	QList<QCameraInfo> cams = QCameraInfo::availableCameras();
	if(cams.empty()){
		printf("camera not found\n");
		return false;
	}

	m_videoSurface = new VideoSurface;
	connect(m_videoSurface, SIGNAL(sendImage(QImage)), this, SLOT(onSendImage(QImage)), Qt::QueuedConnection);

	m_camera = new QCamera(cams[0]);
	m_camera->setCaptureMode(QCamera::CaptureVideo);

	m_camera->setViewfinder(m_videoSurface);

	m_camera->start();

	return true;
}

void CameraStream::imageCaptured(int id, const QImage &preview)
{

}

void CameraStream::onSendImage(const QImage &image)
{
	m_imageSize = image.size();
	m_numImage++;
}

void CameraStream::onTimeout()
{
	printf("image #%d %dx%d               \r", m_numImage, m_imageSize.width(), m_imageSize.height());
}

void CameraStream::run()
{
	initCamera();
	initContext();

	m_timer.reset(new QTimer);
	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(onTimeout()));
	m_timer->start(200);

	exec();

	if(m_camera)
		delete m_camera;

	if(m_videoSurface)
		delete m_videoSurface;

	if(m_fmt){
		avcodec_free_context(&m_fmt);
	}

}

void CameraStream::initContext()
{
	m_codec = avcodec_find_encoder_by_name("libx264");

	if(!m_codec)
		return;

	m_fmt = avcodec_alloc_context3(m_codec);

	AVDictionary *dict = nullptr;
	av_dict_set(&dict, "b", "10M", 0);
	if(avcodec_open2(m_fmt, m_codec, nullptr) < 0){
		avcodec_free_context(&m_fmt);
		m_codec = nullptr;
		return;
	}
}
