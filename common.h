#ifndef COMMON_H
#define COMMON_H

#include <QEvent>
#include <QByteArray>

#include <QImage>

#include <memory>
#include <vector>

class YUVImage{
public:
    YUVImage(){}
    YUVImage(int width, int height, uint8_t *data[8], int linesize[8]){
        set(width, height, data, linesize);
    }

    void set(int width, int height, uint8_t *data[8], int linesize[8]){
        m_width = width;
        m_height = height;

        Y.resize(linesize[0] * height);
        U.resize(linesize[1] * height/2);
        V.resize(linesize[2] * height/2);

        std::copy(data[0], data[0] + Y.size(), Y.data());
        std::copy(data[1], data[1] + U.size(), U.data());
        std::copy(data[2], data[2] + V.size(), V.data());
    }

    int width() const { return m_width; }
    int height() const {return m_height; }
    bool isNull() const { return Y.empty() || U.empty() || V.empty(); }

    void save(const QString& fn){}

    std::vector<uchar> Y;
    std::vector<uchar> U;
    std::vector<uchar> V;
private:
    int m_width     = 0;
    int m_height    = 0;
};

typedef std::shared_ptr<YUVImage> P1Image;
typedef std::shared_ptr<QImage> PImage;

struct Image{
	int width   = 0;
	int height  = 0;
	int linesize[8];
	std::vector< uint8_t > data[8];

	bool empty() const{
		return width == 0 || height == 0 || data[0].empty() || data[1].empty() || data[2].empty();
	}
};

class EventTest: public QEvent{
    Q_GADGET
public:
    enum{EVENT = QEvent::User + 100};

    QByteArray data;
	int tp = 0;

    EventTest(const QByteArray& data): QEvent((Type)EVENT){
        this->data = data;
    }

	EventTest(const QByteArray& data, int tp): QEvent((Type)EVENT){
		this->data = data;
		this->tp = tp;
	}
};

namespace tcp_packet{

/// Tcp packet
/// "VIDEO"|t:uchar|size:uint|data|"PACK"
const static char* begin_packet	= "VIDEO";
const static int len_begin_packet	= sizeof(begin_packet)/sizeof(*begin_packet);
const static char* end_packet		= "PACK";
const static int len_end_packet	= sizeof(end_packet)/sizeof(*end_packet);

const static uchar type_video		= 1;
const static uchar type_control	= 2;
const static uchar type_end		= 10;

union UInt{
	uchar uc[4];
	uint  ui;
};

inline uint getUint(const char* data)
{
	UInt d;
	d.uc[0] = data[0];
	d.uc[1] = data[1];
	d.uc[2] = data[2];
	d.uc[3] = data[3];
	return d.ui;
}

inline UInt getUint(uint val){
	UInt d;
	d.ui = val;
	return d;
}

};

#endif // COMMON_H
