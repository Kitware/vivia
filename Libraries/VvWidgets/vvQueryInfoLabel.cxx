// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vvQueryInfoLabel.h"

#include <QEvent>

#include <qtScopedValueChange.h>
#include <qtStlUtil.h>

#include "vvDescriptorInfoDialog.h"
#include "vvTrackInfoDialog.h"

QTE_IMPLEMENT_D_FUNC(vvQueryInfoLabel)

//-----------------------------------------------------------------------------
class vvQueryInfoLabelPrivate
{
public:
  vvQueryInfoLabelPrivate() : retrievalType(-1) {}

  QString errorText;
  QList<vvTrack> tracks;
  QList<vvDescriptor> descriptors;
  int retrievalType;
};

//-----------------------------------------------------------------------------
vvQueryInfoLabel::vvQueryInfoLabel(QWidget* parent) :
  QLabel(parent),
  d_ptr(new vvQueryInfoLabelPrivate)
{
  connect(this, SIGNAL(linkActivated(QString)),
          this, SLOT(showInfoDialog(QString)));
}

//-----------------------------------------------------------------------------
vvQueryInfoLabel::~vvQueryInfoLabel()
{
}

//-----------------------------------------------------------------------------
QString vvQueryInfoLabel::errorText() const
{
  QTE_D_CONST(vvQueryInfoLabel);
  return d->errorText;
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setErrorText(QString newText)
{
  QTE_D(vvQueryInfoLabel);
  if (d->errorText != newText)
    {
    d->errorText = newText;
    this->updateText();
    }
}

//-----------------------------------------------------------------------------
QList<vvTrack> vvQueryInfoLabel::tracks() const
{
  QTE_D_CONST(vvQueryInfoLabel);
  return d->tracks;
}

//-----------------------------------------------------------------------------
QList<vvDescriptor> vvQueryInfoLabel::descriptors() const
{
  QTE_D_CONST(vvQueryInfoLabel);
  return d->descriptors;
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setQuery(vvSimilarityQuery query)
{
  qtScopedBlockUpdates bu(this);
  this->setTracks(query.Tracks);
  this->setDescriptors(query.Descriptors);
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setQuery(vvQueryInstance query)
{
  QTE_D(vvQueryInfoLabel);

  if (query.isSimilarityQuery())
    {
    this->setQuery(*query.constSimilarityQuery());
    }
  else
    {
    d->retrievalType = -1;
    d->tracks.clear();
    d->descriptors.clear();

    if (query.isRetrievalQuery())
      {
      d->retrievalType = query.retrievalQuery()->RequestedEntities;
      }

    this->updateText();
    }
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setResult(vvQueryResult result)
{
  qtScopedBlockUpdates bu(this);
  this->setTracks(result.Tracks);
  this->setDescriptors(result.Descriptors);
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setTracks(QList<vvTrack> newTracks)
{
  QTE_D(vvQueryInfoLabel);

  d->retrievalType = -1;
  d->tracks = newTracks;

  this->updateText();
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setTracks(std::vector<vvTrack> v)
{
  this->setTracks(qtList(v));
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setDescriptors(QList<vvDescriptor> newDescriptors)
{
  QTE_D(vvQueryInfoLabel);

  d->retrievalType = -1;
  d->descriptors = newDescriptors;

  this->updateText();
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::setDescriptors(std::vector<vvDescriptor> v)
{
  this->setDescriptors(qtList(v));
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::clearData()
{
  qtScopedBlockUpdates bu(this);
  this->setTracks(QList<vvTrack>());
  this->setDescriptors(QList<vvDescriptor>());
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::showTrackInfoDialog()
{
  vvTrackInfoDialog dialog;
  dialog.setTracks(this->tracks());
  dialog.exec();
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::showDescriptorInfoDialog()
{
  vvDescriptorInfoDialog dialog;
  dialog.setDescriptors(this->descriptors());
  dialog.exec();
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::showInfoDialog(QString uri)
{
  if (uri == "t")
    {
    this->showTrackInfoDialog();
    }
  else if (uri == "d")
    {
    this->showDescriptorInfoDialog();
    }
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::updateText()
{
  QTE_D(vvQueryInfoLabel);

  QString t;

  int count = d->descriptors.count();
  if (count)
    {
    const int tcount = d->tracks.count();
    const QString l("<a href=\"%3\">%1 %2</a>");
    QString tl, dl;

    if (tcount)
      {
      tl = l.arg(tcount).arg(tcount > 1 ? "tracks" : "track", "t") + ", ";
      }
    dl = l.arg(count).arg(count > 1 ? "descriptors" : "descriptor", "d");

    t = QString("<html><body>Using %1%2</body></html>").arg(tl, dl);
    }
  else if (d->retrievalType == vvRetrievalQuery::Tracks)
    {
    t = "Track retrieval request";
    }
  else if (d->retrievalType == vvRetrievalQuery::Descriptors)
    {
    t = "Descriptor retrieval request";
    }
  else
    {
    t = "<html><body style=\"color: %1;\">%2</body></html>";
    const QColor color =
      this->palette().color(QPalette::Disabled, this->foregroundRole());
    t = t.arg(color.name(), d->errorText);
    }

  this->setText(t);
}

//-----------------------------------------------------------------------------
void vvQueryInfoLabel::changeEvent(QEvent* e)
{
  QLabel::changeEvent(e);

  if (e->type() == QEvent::PaletteChange)
    {
    this->updateText();
    }
}
