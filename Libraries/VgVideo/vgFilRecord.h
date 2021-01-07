// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgFilRecord_h
#define __vgFilRecord_h

#include <vgExport.h>

#include <QByteArray>
#include <QString>

class VG_VIDEO_EXPORT vgFilRecord
{
public:
  explicit vgFilRecord(const QByteArray&);

  inline operator bool() const { return !this->FramePath.isEmpty(); }

  inline uint frameNumber() const { return this->FrameNumber; }
  inline uint frameWidth() const { return this->FrameWidth; }
  inline uint frameHeight() const { return this->FrameHeight; }
  inline QString framePath() const { return this->FramePath; }

protected:
  uint FrameNumber;
  uint FrameWidth;
  uint FrameHeight;
  QString FramePath;
};

#endif
