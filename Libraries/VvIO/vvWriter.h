// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
