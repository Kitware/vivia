/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgImage.h"

#include <QDebug>
#include <QImage>

#include <limits>

QTE_IMPLEMENT_D_FUNC_SHARED(vgImage)

namespace // anonymous
{

//-----------------------------------------------------------------------------
size_t readData(unsigned char* dest, QDataStream& stream, size_t len)
{
  const int chunk = std::numeric_limits<int>::max();
  size_t read = 0;
  while (len > 0)
    {
    size_t request = qMin(static_cast<size_t>(chunk), len);
    int n = stream.readRawData(reinterpret_cast<char*>(dest),
                               static_cast<int>(request));
    if (n <= 0)
      {
      return read + n;
      }
    read += n;
    dest += n;
    len -= n;
    }
  return read;
}

//-----------------------------------------------------------------------------
size_t writeData(const unsigned char* data, QDataStream& stream, size_t len)
{
  const int chunk = std::numeric_limits<int>::max();
  size_t written = 0;
  while (len > 0)
    {
    size_t request = qMin(static_cast<size_t>(chunk), len);
    int n = stream.writeRawData(reinterpret_cast<const char*>(data),
                                static_cast<int>(request));
    if (n <= 0)
      {
      return written + n;
      }
    written += n;
    data += n;
    len -= n;
    }
  return written;
}

//-----------------------------------------------------------------------------
void arrayDeleter(void* ucharData)
{
  unsigned char* const data = reinterpret_cast<unsigned char*>(ucharData);
  delete[] data;
}

//-----------------------------------------------------------------------------
void qimageDeleter(void* qimagePointer)
{
  QImage* qi = reinterpret_cast<QImage*>(qimagePointer);
  delete qi;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vgImageData : public QSharedData
{
public:
  vgImageData();
  ~vgImageData();

  vgImageData(const vgImageData&);
  vgImageData(const unsigned char* d,
              int ni, int nj, int np,
              int si, int sj, int sp,
              const vgImage::Closure&);

  int iCount, jCount, pCount;
  int iStep, jStep, pStep;
  unsigned char* data;
  vgImage::Closure cleanup;

  static size_t calculateDataSize(int ni, int nj, int np,
                                  int si, int sj, int sp);

protected:
  void setData(const unsigned char* d,
               int ni, int nj, int np,
               int si, int sj, int sp,
               const vgImage::Closure&);
};

//-----------------------------------------------------------------------------
vgImageData::vgImageData() :
  QSharedData(),
  iCount(0), jCount(0), pCount(0),
  iStep(0), jStep(0), pStep(0),
  data(0)
{
}

//-----------------------------------------------------------------------------
vgImageData::vgImageData(const vgImageData& other) : QSharedData(other)
{
  this->setData(other.data, other.iCount, other.jCount, other.pCount,
                other.iStep, other.jStep, other.pStep, vgImage::Closure());
}

//-----------------------------------------------------------------------------
vgImageData::vgImageData(
  const unsigned char* d, int ni, int nj, int np, int si, int sj, int sp,
  const vgImage::Closure& c)
{
  this->setData(d, ni, nj, np, si, sj, sp, c);
}

//-----------------------------------------------------------------------------
vgImageData::~vgImageData()
{
  this->cleanup();
}

//-----------------------------------------------------------------------------
size_t vgImageData::calculateDataSize(int ni, int nj, int np,
                                      int si, int sj, int sp)
{
  if (ni < 0 || nj < 0 || np < 0)
    {
    return 0;
    }

  size_t n = 0;
  n = qMax(n, static_cast<size_t>(ni) * static_cast<size_t>(si));
  n = qMax(n, static_cast<size_t>(nj) * static_cast<size_t>(sj));
  n = qMax(n, static_cast<size_t>(np) * static_cast<size_t>(sp));
  return n;
}

//-----------------------------------------------------------------------------
void vgImageData::setData(
  const unsigned char* d, int ni, int nj, int np, int si, int sj, int sp,
  const vgImage::Closure& c)
{
  this->iCount = ni;
  this->jCount = nj;
  this->pCount = np;
  this->iStep = si;
  this->jStep = sj;
  this->pStep = sp;

  size_t n = calculateDataSize(ni, nj, np, si, sj, sp);

  this->cleanup();
  if (n > 0)
    {
    if (!c)
      {
      this->data = new unsigned char[n];
      qMemCopy(this->data, d, n);
      this->cleanup = vgImage::Closure(arrayDeleter, this->data);
      }
    else
      {
      this->data = const_cast<unsigned char*>(d);
      this->cleanup = c;
      }
    }
  else
    {
    this->data = 0;
    this->cleanup = vgImage::Closure();
    }
}

//-----------------------------------------------------------------------------
vgImage::vgImage() : d_ptr(new vgImageData)
{
}

//-----------------------------------------------------------------------------
vgImage::vgImage(const unsigned char* data, int ni, int nj, int np,
                 int si, int sj, int sp) :
  d_ptr(new vgImageData(data, ni, nj, np, si, sj, sp, Closure()))
{
}

//-----------------------------------------------------------------------------
vgImage::vgImage(unsigned char* data, int ni, int nj, int np,
                 int si, int sj, int sp, const Closure& cleanup) :
  d_ptr(new vgImageData(data, ni, nj, np, si, sj, sp, cleanup))
{
}

//-----------------------------------------------------------------------------
vgImage::vgImage(const QImage& qi)
{
  // Make a copy of the image, coercing the data into a known packing order;
  // this is something of a pessimization based on the assumption that we will
  // eventually need to convert the data to this format anyway, at the expense
  // of slower conversion now... (if the image is already the correct format,
  // this will just be a shallow copy since QImage is implicitly shared, and we
  // need that anyway as we are allowing QImage to retain ownership of the
  // memory)
  QImage* const pqi =
    new QImage(qi.convertToFormat(QImage::Format_RGB888));

  vgImage::Closure cleanup(&qimageDeleter, pqi);
  this->d_ptr = new vgImageData(pqi->constBits(), pqi->width(), pqi->height(),
                                3, 3, pqi->bytesPerLine(), 1, cleanup);
}

//-----------------------------------------------------------------------------
vgImage::~vgImage()
{
}

//-----------------------------------------------------------------------------
vgImage::vgImage(const vgImage& other) : d_ptr(other.d_ptr)
{
}

//-----------------------------------------------------------------------------
vgImage& vgImage::operator=(const vgImage& other)
{
  this->d_ptr = other.d_ptr;
  return *this;
}

//-----------------------------------------------------------------------------
bool vgImage::isValid() const
{
  QTE_D_SHARED(vgImage);
  return d->data;
}

//-----------------------------------------------------------------------------
vgImage::operator bool() const
{
  return this->isValid();
}

//-----------------------------------------------------------------------------
bool vgImage::operator!() const
{
  return !this->isValid();
}

//-----------------------------------------------------------------------------
void vgImage::clear()
{
  if (this->isValid())
    {
    this->d_ptr = new vgImageData;
    }
  else
    {
    this->d_ptr.detach();
    }
}

//-----------------------------------------------------------------------------
const unsigned char* vgImage::constData() const
{
  QTE_D_SHARED(vgImage);
  return d->data;
}

//-----------------------------------------------------------------------------
int vgImage::iCount() const
{
  QTE_D_SHARED(vgImage);
  return d->iCount;
}

//-----------------------------------------------------------------------------
int vgImage::iStep() const
{
  QTE_D_SHARED(vgImage);
  return d->iStep;
}

//-----------------------------------------------------------------------------
int vgImage::jCount() const
{
  QTE_D_SHARED(vgImage);
  return d->jCount;
}

//-----------------------------------------------------------------------------
int vgImage::jStep() const
{
  QTE_D_SHARED(vgImage);
  return d->jStep;
}

//-----------------------------------------------------------------------------
int vgImage::planeCount() const
{
  QTE_D_SHARED(vgImage);
  return d->pCount;
}

//-----------------------------------------------------------------------------
int vgImage::planeStep() const
{
  QTE_D_SHARED(vgImage);
  return d->pStep;
}

//-----------------------------------------------------------------------------
QImage vgImage::toQImage() const
{
  QTE_D_SHARED(vgImage);

  if (d->pCount != 1 && d->pCount != 3)
    {
    // TODO handle other than 1- and 3-plane images?
    qDebug() << "vgImage::toQImage: don't know how to convert image with"
             << d->pCount << "planes";
    return QImage();
    }

  const unsigned char* const rawPixels = d->data;
  QSize const size(d->iCount, d->jCount);

  if (d->pStep == 1 && d->iStep == 3)
    {
    // Trivial case; data is packed RGBRGB...RGB
    return QImage(rawPixels, size.width(), size.height(),
                  d->jStep, QImage::Format_RGB888);
    }

  if (d->pStep == -1 && d->iStep == 4)
    {
    // Trivial case; data is packed BGRxBGRx...BGRx with padding byte 'x'
    return QImage(rawPixels - 2, size.width(), size.height(),
                  d->jStep, QImage::Format_RGB32);
    }

  QImage qi(size, QImage::Format_RGB32);
  const int iStep = d->iStep;
  const int jStep = d->jStep;
  const int planeStep = (d->pCount == 1 ? 0 : d->pStep);
  const int width = size.width();
  QRgb* const newPixels = reinterpret_cast<QRgb*>(qi.bits());

  int y = d->jCount;
  while (y--)
    {
    const unsigned char* const rawScanLine = rawPixels + (y * jStep);
    QRgb* const newScanLine = newPixels + (y * width);

    int x = d->iCount;
    while (x--)
      {
      const unsigned char* const pixoff = rawScanLine + (x * iStep);
      const unsigned char r = *(pixoff + (0 * planeStep));
      const unsigned char g = *(pixoff + (1 * planeStep));
      const unsigned char b = *(pixoff + (2 * planeStep));
      newScanLine[x] = qRgb(r, g, b);
      }
    }
  return qi;
}

//-----------------------------------------------------------------------------
QDataStream& operator>>(QDataStream& stream, vgImage& image)
{
  // Clear image; this will detach so we can use the mutable d-ptr later
  image.clear();
  vgImageData* const d = image.d_func(false);

  // Get image dimensions and pixel strides
  qint32 ni, nj, np, si, sj, sp;
  stream >> ni >> nj >> np >> si >> sj >> sp;

  // Read pixel data
  size_t n = vgImageData::calculateDataSize(ni, nj, np, si, sj, sp);
  if (n > 0)
    {
    QScopedArrayPointer<unsigned char> data(new unsigned char[n]);
    if (readData(data.data(), stream, n) < n)
      {
      return stream;
      }

    // Assign read data to image
    d->data = data.take();
    d->cleanup = vgImage::Closure(arrayDeleter, d->data);
    }

  d->iCount = ni;
  d->jCount = nj;
  d->pCount = np;
  d->iStep = si;
  d->jStep = sj;
  d->pStep = sp;

  return stream;
}

//-----------------------------------------------------------------------------
QDataStream& operator<<(QDataStream& stream, const vgImage& image)
{
  qint32 ni = image.iCount(), nj = image.jCount(), np = image.planeCount();
  qint32 si = image.iStep(), sj = image.jStep(), sp = image.planeStep();

  // Write image dimensions and pixel strides
  stream << ni << nj << np << si << sj << sp;

  // Write pixel data
  size_t n = vgImageData::calculateDataSize(ni, nj, np, si, sj, sp);
  writeData(image.constData(), stream, n);

  return stream;
}
