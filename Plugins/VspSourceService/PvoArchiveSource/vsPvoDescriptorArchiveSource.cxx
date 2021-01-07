// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vsPvoDescriptorArchiveSource.h"

#include <qtKstReader.h>

#include <vsTrackClassifier.h>
#include <vsTrackId.h>

#include <vsAdapt.h>
#include <vsArchiveSourcePrivate.h>

//-----------------------------------------------------------------------------
class vsPvoDescriptorArchiveSourcePrivate : public vsArchiveSourcePrivate
{
public:
  vsPvoDescriptorArchiveSourcePrivate(vsPvoDescriptorArchiveSource* q,
                                      const QUrl& uri);

protected:
  QTE_DECLARE_PUBLIC(vsPvoDescriptorArchiveSource)

  virtual bool processArchive(const QUrl& uri) QTE_OVERRIDE;
};

QTE_IMPLEMENT_D_FUNC(vsPvoDescriptorArchiveSource)

//-----------------------------------------------------------------------------
vsPvoDescriptorArchiveSourcePrivate::vsPvoDescriptorArchiveSourcePrivate(
  vsPvoDescriptorArchiveSource* q, const QUrl& uri) :
  vsArchiveSourcePrivate(q, "descriptors", uri)
{
}

//-----------------------------------------------------------------------------
bool vsPvoDescriptorArchiveSourcePrivate::processArchive(const QUrl& uri)
{
  QTE_Q(vsPvoDescriptorArchiveSource);

  // Read P/V/O's
  qtKstReader reader(uri, QRegExp("\\s+"), QRegExp("\n"));
  if (!reader.isValid())
    {
    return false;
    }

  // Process the P/V/O's
  while (!reader.isEndOfFile())
    {
    int trackId;
    vsTrackObjectClassifier toc;
    if (reader.readInt(trackId, 0) &&
        reader.readReal(toc.probabilityPerson, 1) &&
        reader.readReal(toc.probabilityVehicle, 2) &&
        reader.readReal(toc.probabilityOther, 3))
      {
      emit q->tocAvailable(vsAdaptTrackId(trackId), toc);
      }
    reader.nextRecord();
    }

  // Done
  return true;
}

//-----------------------------------------------------------------------------
vsPvoDescriptorArchiveSource::vsPvoDescriptorArchiveSource(const QUrl& uri) :
  super(new vsPvoDescriptorArchiveSourcePrivate(this, uri))
{
}

//-----------------------------------------------------------------------------
vsPvoDescriptorArchiveSource::~vsPvoDescriptorArchiveSource()
{
}
