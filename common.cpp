
#include "common.h"

inline int clip(int val)
{
    if(val < 0) return 0;
    if(val > 255) return 255;
    return val;
}

void cnvrgb2yuv(QRgb rgb, uchar &y, uchar &u, uchar &v)
{
    int r = qRed(rgb), g = qGreen(rgb), b = qBlue(rgb);
    int Y = 66 * r + 129 * g + 25 * b;
    int U = -38 * r - 74 * g + 112 * b;
    int V = 112 * r - 94  *g - 18 * b;

    y = ((Y + 128) >> 8) + 16;
    u = ((U + 128) >> 8) + 128;
    v = ((V + 128) >> 8) + 128;
}

void YUVImage::createFromQImage(const QImage &image)
{
    if(image.isNull())
        return;

    m_width = image.width();
    m_height = image.height();

    int lY = m_width * m_height;
    int lU = m_width/2 * m_height/2;
    int lV = lU;

    Y.resize(lY);
    U.resize(lU);
    V.resize(lV);

//#pragma omp parallel for
    for(int i = 0; i < image.height()/2; ++i){
        QRgb* sc0 = (QRgb*)image.scanLine(i * 2 + 0);
        QRgb* sc1 = (QRgb*)image.scanLine(i * 2 + 1);

        uchar *py0 = &Y.data()[(i * 2 + 0) * m_width];
        uchar *py1 = &Y.data()[(i * 2 + 1) * m_width];

        uchar *pu = &U.data()[i * m_width/2];
        uchar *pv = &V.data()[i * m_width/2];

        for(int j = 0; j < image.width()/2; ++j){
            QRgb rgb00 = sc0[2 * j + 0];
            QRgb rgb01 = sc0[2 * j + 1];
            QRgb rgb10 = sc1[2 * j + 0];
            QRgb rgb11 = sc1[2 * j + 1];

            uchar y, u, v;

            cnvrgb2yuv(rgb00, y, u, v);
            py0[2 * j + 0] = y;
            cnvrgb2yuv(rgb01, y, u, v);
            py0[2 * j + 1] = y;
            cnvrgb2yuv(rgb10, y, u, v);
            py1[2 * j + 0] = y;
            cnvrgb2yuv(rgb11, y, u, v);
            py1[2 * j + 1] = y;

            pu[j] = u;
            pv[j] = v;
        }
    }
}
