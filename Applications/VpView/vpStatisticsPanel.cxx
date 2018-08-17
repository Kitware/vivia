/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpStatisticsPanel.h"

#include "vpViewCore.h"
#include "vtkVpTrackModel.h"

#include <vtkVgTrackFilter.h>
#include <vtkVgTrackTypeRegistry.h>

#include <qtEnumerate.h>
#include <qtIndexRange.h>

#include <QFormLayout>
#include <QHash>
#include <QLabel>
#include <QSet>

//-----------------------------------------------------------------------------
class vpStatisticsPanelPrivate
{
public:
  vpViewCore* core = nullptr;
  QList<int> trackTypes;

  QFormLayout* layout = nullptr;
  QHash<int, QLabel*> nameLabels;
  QHash<int, QLabel*> countLabels;

  bool dirty = false;
};

QTE_IMPLEMENT_D_FUNC(vpStatisticsPanel)

//-----------------------------------------------------------------------------
vpStatisticsPanel::vpStatisticsPanel(QWidget* parent)
  : QWidget{parent}, d_ptr{new vpStatisticsPanelPrivate}
{
  QTE_D();
  d->layout = new QFormLayout;
  this->setLayout(d->layout);
}

//-----------------------------------------------------------------------------
vpStatisticsPanel::~vpStatisticsPanel()
{
}

//-----------------------------------------------------------------------------
void vpStatisticsPanel::bind(vpViewCore* core)
{
  QTE_D();
  d->core = core;
}

//-----------------------------------------------------------------------------
void vpStatisticsPanel::invalidate()
{
  QTE_D();
  if (!d->dirty)
  {
    d->dirty = true;
    QMetaObject::invokeMethod(this, "updateStatistics", Qt::QueuedConnection);
  }
}

//-----------------------------------------------------------------------------
void vpStatisticsPanel::updateStatistics()
{
  QTE_D();

  auto* const ttr = d->core->getTrackTypeRegistry();
  QList<int> newTrackTypes;
  for (auto tti : qtIndexRange(ttr->GetNumberOfTypes()))
  {
    // TODO only count types that are "used"? (But that is none of them!)
    newTrackTypes.append(tti);
  }

  if (newTrackTypes != d->trackTypes)
  {
    qDeleteAll(d->nameLabels);
    qDeleteAll(d->countLabels);
    d->nameLabels.clear();
    d->countLabels.clear();

    for (auto tti : newTrackTypes)
    {

      auto* const nameLabel = new QLabel{this};
      auto* const countLabel = new QLabel{this};

      nameLabel->setText(QString::fromLocal8Bit(ttr->GetType(tti).GetName()));

      d->layout->addRow(nameLabel, countLabel);

      d->nameLabels.insert(tti, nameLabel);
      d->countLabels.insert(tti, countLabel);
    }

    d->trackTypes = newTrackTypes;
  }

  auto* const tf = d->core->getTrackFilter();
  QHash<int, int> trackCounts;
  for (auto si : qtIndexRange(d->core->sessionCount()))
  {
    if (d->core->isSessionEnabled(si))
    {
      auto* const model = d->core->getTrackModel(si);
      model->InitTrackTraversal();
      while (auto&& ti = model->GetNextTrack())
      {
        const auto tc = tf->GetBestClassifier(ti.GetTrack());
        if (tc != -1)
        {
          ++trackCounts[tc];
        }
      }
    }
  }

  for (auto iter : qtEnumerate(d->countLabels))
  {
    const auto count = trackCounts.value(iter.key(), 0);
    iter.value()->setText(QString::number(count));
  }

  d->dirty = false;
}
