// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpQtSceneUtils.h"

// Qt includes
#include <QComboBox>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>

// VTK includes
#include <vtkVgBaseImageSource.h>

//-----------------------------------------------------------------------------
void vpQtSceneUtils::addContextLevels(vtkVgBaseImageSource* contextSource, QComboBox* comboBox)
{
  if (!contextSource || !comboBox)
    {
    qDebug() << "ERROR: Invalid (NULL) context source or combo box";
    return;
    }

  int countContextLOD = contextSource->GetNumberOfLevels();

  comboBox->blockSignals(true);

  comboBox->addItem(QString("Auto"), QVariant(-1));

  // \note: the base level starts at 0.
  for (int i = 0; i < countContextLOD; ++i)
    {
    QString level("Level_");
    level.append(QString("%1").arg(i));
    comboBox->addItem(
      level, QVariant(i));
    }

  comboBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
QWidget* vpQtSceneUtils::createContextSlider(QSlider* slider, QWidget* parent/*=0*/)
{
  QWidget* widget = new QWidget(parent);
  QGridLayout* layout = new QGridLayout(widget);
  QLabel* mainLabel = new QLabel("Context Detail", parent);
  QLabel* resolutionLabel = new QLabel("Low", parent);
  layout->addWidget(resolutionLabel, 0, 0);
  layout->addWidget(slider, 0, 1);
  resolutionLabel = new QLabel("High", parent);
  layout->addWidget(resolutionLabel, 0, 2);
  mainLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(mainLabel, 1, 1);
  return widget;
}
