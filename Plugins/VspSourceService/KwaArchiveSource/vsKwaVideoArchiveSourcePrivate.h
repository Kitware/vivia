// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
