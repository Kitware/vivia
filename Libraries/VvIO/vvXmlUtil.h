// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvXmlUtil_h
#define __vvXmlUtil_h

#include <vgExport.h>

#include "vvWriterData.h"

class QDomDocument;
class QDomNode;

#define OUT_ERROR QString* error = 0

namespace vvXmlUtil
{
  VV_IO_EXPORT bool readNode(const QDomNode&, vvTrack&, OUT_ERROR);
  VV_IO_EXPORT bool readNode(const QDomNode&, vvDescriptor&, OUT_ERROR);
  VV_IO_EXPORT bool readNode(const QDomNode&, vvQueryInstance&, OUT_ERROR);
  VV_IO_EXPORT bool readNode(const QDomNode&, vvQueryResult&, OUT_ERROR);
  VV_IO_EXPORT bool readNode(const QDomNode&, vvEventSetInfo&, OUT_ERROR);
  VV_IO_EXPORT bool readNode(const QDomNode&, vgGeocodedPoly&,
                             vvDatabaseQuery::IntersectionType*, OUT_ERROR);

  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvTrack&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvDescriptor&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvDatabaseQuery&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvRetrievalQuery&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvSimilarityQuery&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvQueryResult&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vgGeocodedPoly&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvSpatialFilter&);
  VV_IO_EXPORT QDomNode makeNode(QDomDocument&, const vvEventSetInfo&);
}

#undef OUT_ERROR

#endif
