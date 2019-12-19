#include "videosurface.h"

VideoSurface::VideoSurface(QObject *parent)
	: QAbstractVideoSurface(parent)
{

}


QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
	QList<QVideoFrame::PixelFormat> fmt;
	fmt.push_back(QVideoFrame::Format_RGB32);
	fmt.push_back(QVideoFrame::Format_ARGB32);
	return fmt;
}

bool VideoSurface::start(const QVideoSurfaceFormat &format)
{
	m_fmt = format;

	QAbstractVideoSurface::start(format);

	return true;
}

void VideoSurface::stop()
{
	QAbstractVideoSurface::stop();
}

bool VideoSurface::present(const QVideoFrame &frame)
{
	if(!frame.isValid())
		return false;

	QVideoFrame c_frame(frame);

	int w = frame.width();
	int h = frame.height();
	int fmt = frame.pixelFormat();

	const uchar *bits = c_frame.bits();

	int mb = c_frame.mappedBytes();
	int pc = c_frame.planeCount();
	int bpw = c_frame.bytesPerLine();

	int c = 4;

	QImage::Format imageFormat = QImage::Format_ARGB32;
	if(frame.pixelFormat() == QVideoFrame::Format_ARGB32 || frame.pixelFormat() == QVideoFrame::Format_RGB32
			|| frame.pixelFormat() == QVideoFrame::Format_BGRA32 || frame.pixelFormat() == QVideoFrame::Format_BGR32){
		c = 4;
	}
	if(frame.pixelFormat() == QVideoFrame::Format_RGB24 || frame.pixelFormat() == QVideoFrame::Format_BGR24){
		c = 3;
		imageFormat = QImage::Format_RGB888;
	}

	QImage image(
				c_frame.bits(),
				w, h,
				c_frame.bytesPerLine(),
				imageFormat);

	emit sendImage(image);

	return true;
}

bool VideoSurface::isFormatSupported(const QVideoSurfaceFormat &format) const
{
	QVideoSurfaceFormat fmt = format;
	return true;
}
