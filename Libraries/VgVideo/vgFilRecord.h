/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
