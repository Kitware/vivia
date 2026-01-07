// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsVideoHelper_h
#define __vsVideoHelper_h

#include <QObject>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vgNamespace.h>

struct vgTimeStamp;
class  vgImage;
class  vgVideo;
class  vgVideoFramePtr;
struct vgVideoSeekRequest;
class  vgVideoSourceRequestor;

class vsVideoHelperPrivate;

class VSP_SOURCEUTIL_EXPORT vsVideoHelper : public QObject
{
  Q_OBJECT

public:
  explicit vsVideoHelper(QObject* parent = 0);
  virtual ~vsVideoHelper();

  vgVideoFramePtr updateFrame(const vgVideo& video,
                              const vgVideoSeekRequest& request,
                              vgImage& image);
  void clearLastRequest(vgVideoSourceRequestor*);

  static vgTimeStamp findTime(const vgVideo& video, unsigned int frameNumber,
                              vg::SeekMode roundMode);

protected slots:
  void cleanupRequestor(QObject*);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsVideoHelper)

private:
  QTE_DECLARE_PRIVATE(vsVideoHelper)
};

#endif
