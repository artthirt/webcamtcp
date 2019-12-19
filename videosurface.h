#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QImage>

class VideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT
public:
	VideoSurface(QObject *parent = nullptr);
	bool isFormatSupported(const QVideoSurfaceFormat &format) const;
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
	bool start(const QVideoSurfaceFormat &format);
	void stop();
	bool present(const QVideoFrame &frame);

signals:
	void sendImage(QImage);

private:
	QVideoSurfaceFormat m_fmt;

};

#endif // VIDEOSURFACE_H
