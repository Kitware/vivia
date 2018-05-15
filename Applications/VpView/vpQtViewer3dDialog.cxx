/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpQtViewer3dDialog.h"
#include "ui_vpQtViewer3dDialog.h"

// Qt includes
#include <QDebug>

// VisGUI includes.
#include "vtkVgBaseImageSource.h"

#include "vpQtSceneUtils.h"
#include "vpViewCore.h"

//-----------------------------------------------------------------------------
class vpQtViewer3dDialog::vtkInternal
{
public:

  vtkInternal()
    {
    this->UI = new Ui::vpQtViewer3dDialog();
    }

  ~vtkInternal()
    {
    delete this->UI;
    }

  Ui::vpQtViewer3dDialog* UI;

  QWidget* ContextLODWidget;
};

//-----------------------------------------------------------------------------
vpQtViewer3dDialog::vpQtViewer3dDialog(QWidget* parent) :
  QDialog(parent)
{
  this->Internal = new vtkInternal();
  this->Internal->UI->setupUi(this);

  this->setupInternalUi();

  connect(this->Internal->UI->edgeColorModeComboBox,
          SIGNAL(currentIndexChanged(int)),
          this->Internal->UI->viewer3dWidget->getViewer3d(),
          SLOT(setGraphEdgeColorMode(int)));

  connect(this->Internal->UI->edgeThicknessModeComboBox,
          SIGNAL(currentIndexChanged(int)),
          this->Internal->UI->viewer3dWidget->getViewer3d(),
          SLOT(setGraphEdgeThicknessMode(int)));

  connect(this->Internal->UI->nodeHeightComboBox,
          SIGNAL(currentIndexChanged(int)),
          this->Internal->UI->viewer3dWidget->getViewer3d(),
          SLOT(setGraphNodeHeightMode(int)));

  connect(this->Internal->UI->viewer3dWidget->getViewer3d(),
          SIGNAL(contextCreated()),
          this, SLOT(onContextCreated()));
}

//-----------------------------------------------------------------------------
vpQtViewer3dDialog::~vpQtViewer3dDialog()
{
  delete this->Internal;
  this->Internal = 0;
}

//-----------------------------------------------------------------------------
vpQtViewer3dWidget* vpQtViewer3dDialog::getViewer3dWidget()
{
  return this->Internal->UI->viewer3dWidget;
}

//-----------------------------------------------------------------------------
void vpQtViewer3dDialog::update(const vtkVgTimeStamp& timestamp)
{
  this->Internal->UI->viewer3dWidget->update(timestamp);
}

//-----------------------------------------------------------------------------
void vpQtViewer3dDialog::reset()
{
  this->Internal->UI->viewer3dWidget->reset();
}

//-----------------------------------------------------------------------------
void vpQtViewer3dDialog::onContextCreated()
{
  vtkVgBaseImageSource* contextSource =
    this->Internal->UI->viewer3dWidget->getViewer3d()->getContextSource();

  if (contextSource->GetNumberOfLevels() > 1)
    {
    this->Internal->ContextLODWidget->setEnabled(true);
    }
  else
    {
    this->Internal->ContextLODWidget->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3dDialog::onContextLODChanged(int value)
{
  this->Internal->UI->viewer3dWidget->getViewer3d()->setContextLevelOfDetailFactor(
    value);

  this->Internal->UI->viewer3dWidget->getViewer3d()->forceRender();
}

//-----------------------------------------------------------------------------
void vpQtViewer3dDialog::setupInternalUi()
{
  // Construct context LOD slider.
  QHBoxLayout* contextLODLayout = new QHBoxLayout();
  QSlider* contextLODSlider =
    new QSlider(Qt::Horizontal, this->Internal->UI->sceneGroupBox);
  contextLODSlider->setRange(-10, 10);
  contextLODSlider->setTickInterval(1);
  contextLODSlider->setSizePolicy(QSizePolicy::Preferred,
                                  QSizePolicy::Preferred);

  this->Internal->ContextLODWidget =
    vpQtSceneUtils::createContextSlider(contextLODSlider,
                                        this->Internal->UI->sceneGroupBox);

  contextLODLayout->addWidget(this->Internal->ContextLODWidget);

  this->Internal->UI->sceneGroupBox->setLayout(contextLODLayout);

  connect(contextLODSlider, SIGNAL(valueChanged(int)) , this,
          SLOT(onContextLODChanged(int)));

  // Edge color modes.
  QStringList edgeColorModes;
  this->Internal->UI->viewer3dWidget->getViewer3d()->getGraphEdgeColorModes(
    edgeColorModes);

  foreach (const auto& edgeColorMode, edgeColorModes)
    {
    this->Internal->UI->edgeColorModeComboBox->addItem(edgeColorMode);
    }

  // Edge thickness modes.
  QStringList edgeThicknessModes;
  this->Internal->UI->viewer3dWidget->getViewer3d()->getGraphEdgeThicknessModes(
    edgeThicknessModes);

  foreach (const auto& edgeThicknessMode, edgeThicknessModes)
    {
    this->Internal->UI->edgeThicknessModeComboBox->addItem(edgeThicknessMode);
    }
}
