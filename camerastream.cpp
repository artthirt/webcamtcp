#include "camerastream.h"

#include <QList>
#include <QCameraInfo>

#include <chrono>

CameraStream::CameraStream(QObject *parent) : QThread(parent)
{
	m_camera = nullptr;
	m_videoSurface = nullptr;
	m_numImage = 0;
	m_isInitAV = false;
}

CameraStream::~CameraStream()
{
    m_done = true;

    if(m_encoderThread.get()){
        m_encoderThread->join();
    }

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
	connect(m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(stateChanged(QCamera::State)));
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

    if(m_fmt){
        if(m_frames.size() < m_max_frames){
            m_frames.push(image);
        }
    }
}

void CameraStream::onTimeout()
{
	printf("image #%d %dx%d               \r", m_numImage, m_imageSize.width(), m_imageSize.height());
	if(!m_isInitAV){
		if(!m_imageSize.isEmpty()){
			initContext(m_imageSize.width(), m_imageSize.height());
			m_isInitAV = true;
		}
	}
}

void CameraStream::stateChanged(QCamera::State state)
{
	if(state == QCamera::ActiveState){
	}
}

void CameraStream::run()
{
    //initContext(640, 480);
	initCamera();

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

void CameraStream::initContext(int width, int height)
{
    m_codec = avcodec_find_encoder(AV_CODEC_ID_H264);

    if(!m_codec)
        m_codec = avcodec_find_encoder_by_name("libx264");

	if(!m_codec)
        m_codec = avcodec_find_encoder_by_name("h264_omx");

	if(!m_codec)
		return;

	if(m_fmt){
		avcodec_free_context(&m_fmt);
		m_fmt = nullptr;
	}

	m_fmt = avcodec_alloc_context3(m_codec);
//	m_fmt->codec_type = AVMEDIA_TYPE_VIDEO;
//	m_fmt->codec_id = m_codec->id;
//	m_fmt->bit_rate = 10000000;
//	m_fmt->flags = AV_CODEC_FLAG_GLOBAL_HEADER;
    m_fmt->time_base = {1, 25};
    m_fmt->gop_size = 25;
    m_fmt->keyint_min = 2;
    m_fmt->framerate = {25, 1};
    m_fmt->ticks_per_frame = 2;
    m_fmt->pix_fmt = AV_PIX_FMT_YUV420P;

	m_fmt->width = width;
	m_fmt->height = height;

	AVDictionary *dict = nullptr;
	av_dict_set(&dict, "b", "10M", 0);
	av_dict_set(&dict, "r", "25", 0);
	av_dict_set(&dict, "c", "v", 0);
    //av_dict_set(&dict, "scale", QString("%1:%2").arg(width).arg(height).toLatin1().data(), 0);
	int res = 0;
    if((res = avcodec_open2(m_fmt, m_codec, &dict)) < 0){
		char errbuf[128] = {0};
		av_make_error_string(errbuf, 128, res);
		printf("open encoder error %s", errbuf);
		avcodec_free_context(&m_fmt);
		m_codec = nullptr;
		return;
    }

    m_encoderThread.reset(new std::thread(std::bind(&CameraStream::doEncode, this)));
}

void CameraStream::doEncode()
{
    while(!m_done){
        if(m_frames.empty()){
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }else{
            QImage im = m_frames.front();
            m_frames.pop();
            encodeFrame(im);
        }
    }
}

void CameraStream::encodeFrame(const QImage &image)
{
    AVFrame *frame = av_frame_alloc();

    int res = 0;
    do{
        res = avcodec_send_frame(m_fmt, frame);
    }while(res == AVERROR(EAGAIN));

    AVPacket pkt;
    std::fill((char*)&pkt, (char*)&pkt + sizeof(AVPacket), 0);

    QByteArray data;

    while(avcodec_receive_packet(m_fmt, &pkt) >= 0){

        data.resize(pkt.size);
        std::copy(pkt.data, pkt.data + pkt.size, data.data());
        emit sendPacket(data);

        av_packet_unref(&pkt);
    }

    av_frame_free(&frame);
}

