/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
