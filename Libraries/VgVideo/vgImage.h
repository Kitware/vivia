// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgImage_h
#define __vgImage_h

#include <QDataStream>
#include <QExplicitlySharedDataPointer>

#include <qtGlobal.h>

#include <vgExport.h>

class QImage;

class vgImageData;

class VG_VIDEO_EXPORT vgImage
{
public:
  typedef void (*Deleter)(void*);

  struct Closure
    {
      inline Closure(Deleter function = 0, void* data = 0) :
        CleanupFunction(function), CleanupData(data)
        {}

      inline operator bool() const { return this->CleanupFunction; }
      inline bool operator!() const { return !this->CleanupFunction; }

      inline void operator()() const
        {
        if (this->CleanupFunction)
          {
          (*this->CleanupFunction)(this->CleanupData);
          }
        }

    protected:
      friend class vgImageData;

      Deleter CleanupFunction;
      void* CleanupData;
    };

  vgImage();
  vgImage(const unsigned char* data,
          int ni, int nj, int np,
          int si, int sj, int sp);
  vgImage(unsigned char* data,
          int ni, int nj, int np,
          int si, int sj, int sp,
          const Closure& cleanup = Closure());
  vgImage(const QImage&);
  ~vgImage();

  vgImage(const vgImage&);
  vgImage& operator=(const vgImage&);

  bool isValid() const;
  operator bool() const;
  bool operator!() const;

  void clear();

  const unsigned char* constData() const;

  int iCount() const;
  int iStep() const;

  int jCount() const;
  int jStep() const;

  int planeCount() const;
  int planeStep() const;

  QImage toQImage() const;

protected:
  QTE_DECLARE_SHARED_EPTR(vgImage)

  friend VG_VIDEO_EXPORT QDataStream& operator>>(QDataStream&, vgImage&);

private:
  QTE_DECLARE_SHARED(vgImage)
};

extern VG_VIDEO_EXPORT QDataStream& operator>>(QDataStream&, vgImage&);
extern VG_VIDEO_EXPORT QDataStream& operator<<(QDataStream&, const vgImage&);

#endif
