/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsKwaVideoArchiveSourcePrivate_h
#define __vsKwaVideoArchiveSourcePrivate_h

#include <vsVideoSourcePrivate.h>

#include <vsVideoArchive.h>

class vsKwaVideoArchiveSourcePrivate : public vsVideoSourcePrivate
{
public:
  vsKwaVideoArchiveSourcePrivate(vsVideoSource* q, QUrl archiveUri);
  virtual ~vsKwaVideoArchiveSourcePrivate();

protected:
  friend class vsKwaVideoArchiveSource;

  virtual void run() QTE_OVERRIDE;
  virtual void findTime(vtkVgTimeStamp*, unsigned int,
                        vg::SeekMode) QTE_OVERRIDE;
  virtual void requestFrame(const vgVideoSeekRequest&) QTE_OVERRIDE;
  virtual void clearLastRequest(vgVideoSourceRequestor*) QTE_OVERRIDE;

  vsVideoArchive Archive;
  int StatusTimerId;
  int StatusTtl;

private:
  QTE_DISABLE_COPY(vsKwaVideoArchiveSourcePrivate)
};

#endif
