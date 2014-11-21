/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsVideoArchive_h
#define __vsVideoArchive_h

#include <QList>
#include <QObject>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vgNamespace.h>

#include <vtkVgTimeStamp.h>

#include <vtkVgVideoFrameMetaData.h>

#include <vgVideoSourceRequestor.h>

class QUrl;

class vsVideoArchivePrivate;

class VSP_SOURCEUTIL_EXPORT vsVideoArchive : public QObject
{
  Q_OBJECT

public:
  vsVideoArchive(const QUrl& archiveUri);
  virtual ~vsVideoArchive();

  virtual bool okay() const;
  virtual QUrl uri() const;

  virtual void initialize();
  virtual void requestFrame(vgVideoSeekRequest);
  virtual void clearLastRequest(vgVideoSourceRequestor*);
  virtual vtkVgTimeStamp findTime(unsigned int frameNumber,
                                  vg::SeekMode roundMode);

signals:
  void frameRangeAvailable(vtkVgTimeStamp first, vtkVgTimeStamp last);
  void metadataAvailable(QList<vtkVgVideoFrameMetaData> metadata);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsVideoArchive)

private:
  QTE_DECLARE_PRIVATE(vsVideoArchive)
  QTE_DISABLE_COPY(vsVideoArchive)
};

#endif
