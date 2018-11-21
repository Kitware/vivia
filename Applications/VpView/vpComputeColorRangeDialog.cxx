/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpComputeColorRangeDialog.h"
#include "ui_vpComputeColorRangeDialog.h"

#include "vpViewCore.h"

#include <QProgressDialog>
#include <QPushButton>

QTE_IMPLEMENT_D_FUNC(vpComputeColorRangeDialog)

//-----------------------------------------------------------------------------
class vpComputeColorRangeDialogPrivate
{
public:
  Ui::vpComputeColorRangeDialog UI;
  vpViewCore* Core;
};

//-----------------------------------------------------------------------------
vpComputeColorRangeDialog::vpComputeColorRangeDialog(
                                     QWidget* parent, vpViewCore* core)
  : QDialog(parent), d_ptr(new vpComputeColorRangeDialogPrivate)
{
  QTE_D(vpComputeColorRangeDialog);

  d->Core = core;
  d->UI.setupUi(this);

  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

  connect(d->UI.computeCurrentFrame, SIGNAL(clicked()),
          this, SLOT(computeFrameColorRange()));
  connect(d->UI.computeAllFrames, SIGNAL(clicked()),
    this, SLOT(computeAllFramesColorRange()));
}

//-----------------------------------------------------------------------------
vpComputeColorRangeDialog::~vpComputeColorRangeDialog()
{
}

//-----------------------------------------------------------------------------
void vpComputeColorRangeDialog::computeFrameColorRange()
{
  QTE_D_CONST(vpComputeColorRangeDialog);

  double* range = d->Core->getCurrentFrameColorScalarRange();
  d->UI.minValue->setText(QString::number(range[0]));
  d->UI.maxValue->setText(QString::number(range[1]));

  this->ComputedWidth = range[1] - range[0];
  this->ComputedCenter = (range[1] + range[0]) / 2.0;

  d->UI.widthValue->setText(QString::number(this->ComputedWidth));
  d->UI.centerValue->setText(QString::number(this->ComputedCenter));

  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
}

//-----------------------------------------------------------------------------
void vpComputeColorRangeDialog::computeAllFramesColorRange()
{
  QTE_D_CONST(vpComputeColorRangeDialog);

  int numberOfFrames = d->Core->getNumberOfFrames();
  QProgressDialog progress("Computing Color Range Across All Images...", "Cancel",
    0, numberOfFrames);
  progress.setWindowModality(Qt::ApplicationModal);

  // Manually force the dialog to be shown. Otherwise, in release builds a
  // strange bug can occur where the dialog never appears, and allows the user
  // to continue interacting with the UI.
  progress.show();

  double minMin = VTK_DOUBLE_MAX;
  double maxMax = VTK_DOUBLE_MIN;

  // Visit all the frames but want to quickly scan the "entire" range
  int frameSkip = 
    static_cast<int>(ceil(static_cast<float>(numberOfFrames) / 8.0f));
  int processedCount = 0;
  
  for (int offset = 0; offset < frameSkip; offset++)
    {
    for (int frameIndex = offset; frameIndex < numberOfFrames;
         frameIndex += frameSkip)
      {
      double* range = d->Core->getFrameColorScalarRange(frameIndex);
      if (range[0] < minMin)
        {
        minMin = range[0];
        }
      if (range[1] > maxMax)
        {
        maxMax = range[1];
        }

      d->UI.minValue->setText(QString::number(minMin));
      d->UI.maxValue->setText(QString::number(maxMax));

      this->ComputedWidth = maxMax - minMin;
      this->ComputedCenter = (maxMax + minMin) / 2.0;

      d->UI.widthValue->setText(QString::number(this->ComputedWidth));
      d->UI.centerValue->setText(QString::number(this->ComputedCenter));
      progress.setValue(++processedCount);

      QApplication::sendPostedEvents();
      QApplication::processEvents();

      if (progress.wasCanceled())
        {
        progress.setValue(numberOfFrames);
        d->UI.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
        return;
        }
      }
    }

  progress.setValue(numberOfFrames);

  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
}
