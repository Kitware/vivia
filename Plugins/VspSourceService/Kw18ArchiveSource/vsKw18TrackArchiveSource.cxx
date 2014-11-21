/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsKw18TrackArchiveSource.h"

#include <qtThread.h>

#include <tracking/kw18_reader.h>

#include <vsAdapt.h>
#include <vsArchiveSourcePrivate.h>

#include <vvAdaptVidtk.h>

//-----------------------------------------------------------------------------
class vsKw18TrackArchiveSourcePrivate : public vsArchiveSourcePrivate
{
public:
  vsKw18TrackArchiveSourcePrivate(vsKw18TrackArchiveSource* q, const QUrl& uri);

protected:
  QTE_DECLARE_PUBLIC(vsKw18TrackArchiveSource)

  virtual bool processArchive(const QUrl& uri) QTE_OVERRIDE;
};

QTE_IMPLEMENT_D_FUNC(vsKw18TrackArchiveSource)

//-----------------------------------------------------------------------------
vsKw18TrackArchiveSourcePrivate::vsKw18TrackArchiveSourcePrivate(
  vsKw18TrackArchiveSource* q, const QUrl& uri)
  : vsArchiveSourcePrivate(q, "tracks", uri)
{
}

//-----------------------------------------------------------------------------
bool vsKw18TrackArchiveSourcePrivate::processArchive(const QUrl& uri)
{
  QTE_Q(vsKw18TrackArchiveSource);

  QString fileName = uri.toLocalFile();
  vidtk::kw18_reader trackReader(qPrintable(fileName));

  // Read tracks
  vcl_vector<vidtk::track_sptr> tracks;
  if (!trackReader.read(tracks))
    {
    return false;
    }

  // Process the tracks
  for (size_t n = 0; n < tracks.size(); ++n)
    {
    QList<vvTrackState> states;
    vvTrackId id = vsAdaptTrackId(tracks[n]->id());
    const vcl_vector<vidtk::track_state_sptr>& h = tracks[n]->history();
    for (size_t j = 0; j < h.size(); ++j)
      {
      vidtk::track_state_sptr s = h[j];
      states.append(vvAdapt(*s));
      }
    emit q->trackUpdated(id, states);
    emit q->trackClosed(id);
    }

  // Done
  return true;
}

//-----------------------------------------------------------------------------
vsKw18TrackArchiveSource::vsKw18TrackArchiveSource(const QUrl& uri) :
  super(new vsKw18TrackArchiveSourcePrivate(this, uri))
{
}

//-----------------------------------------------------------------------------
vsKw18TrackArchiveSource::~vsKw18TrackArchiveSource()
{
}
