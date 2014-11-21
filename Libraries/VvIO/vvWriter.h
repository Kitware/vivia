/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvWriter_h
#define __vvWriter_h

#include <QFile>
#include <QTextStream>

#include <qtGlobal.h>

#include <vgExport.h>

#include "vvWriterData.h"

class vvWriterPrivate;

class VV_IO_EXPORT vvWriter
{
public:
  enum Format
    {
    Kst,
    Xml
    };

  explicit vvWriter(QFile&, Format format = Kst, bool pretty = true);
  explicit vvWriter(QTextStream&, Format format = Kst, bool pretty = true);
  virtual ~vvWriter();

  virtual vvWriter& operator<<(vvHeader::FileType);

  virtual vvWriter& operator<<(const vvTrack&);
  virtual vvWriter& operator<<(const vvDescriptor&);
  virtual vvWriter& operator<<(const vvQueryInstance&);
  virtual vvWriter& operator<<(const vvRetrievalQuery&);
  virtual vvWriter& operator<<(const vvSimilarityQuery&);
  virtual vvWriter& operator<<(const vvQueryResult&);
  virtual vvWriter& operator<<(const vgGeocodedPoly&);
  virtual vvWriter& operator<<(const vvSpatialFilter&);
  virtual vvWriter& operator<<(const vvEventSetInfo&);

  template <typename T>
  vvWriter& operator<<(const QList<T>& list)
    { foreach (const T& item, list) *this << item; return *this; }

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvWriter)

  vvWriter();

private:
  QTE_DECLARE_PRIVATE(vvWriter)
  Q_DISABLE_COPY(vvWriter)
};

#endif
