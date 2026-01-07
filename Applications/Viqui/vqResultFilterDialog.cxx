// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqResultFilterDialog.h"
#include "ui_resultFilter.h"

#include <qtUtil.h>

#include "vqSettings.h"

QTE_IMPLEMENT_D_FUNC(vqResultFilterDialog)

//-----------------------------------------------------------------------------
class vqResultFilterDialogPrivate
{
public:
  Ui::vqResultFilterDialog UI;
};

//-----------------------------------------------------------------------------
vqResultFilterDialog::vqResultFilterDialog(
  QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags), d_ptr(new vqResultFilterDialogPrivate)
{
  QTE_D(vqResultFilterDialog);

  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Connect signals/slots
  connect(d->UI.clear, SIGNAL(clicked(bool)),
          this, SLOT(clearFilters()));
  connect(d->UI.thresholdPreset, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setThresholdFromPreset(int)));

  // Load threshold filter presets
  foreach (vvScoreGradient::Stop stop, vqSettings().scoreGradient().stops())
    d->UI.thresholdPreset->addItem(stop.text, stop.threshold);

  d->UI.thresholdPreset->addItem("Custom", -1.0);
  d->UI.thresholdPreset->setCurrentIndex(0);

  // Adjust checkbox alignment for style
  if (d->UI.formLayout->labelAlignment().testFlag(Qt::AlignRight))
    {
    d->UI.threshold->setLayoutDirection(Qt::RightToLeft);
    }
}

//-----------------------------------------------------------------------------
vqResultFilterDialog::~vqResultFilterDialog()
{
}

//-----------------------------------------------------------------------------
vqResultFilter vqResultFilterDialog::filter() const
{
  QTE_D_CONST(vqResultFilterDialog);

  vqResultFilter result;

  result.Threshold =
    (d->UI.threshold->isChecked() ? d->UI.thresholdValue->value() : -1.0);

  return result;
}

//-----------------------------------------------------------------------------
void vqResultFilterDialog::setFilter(const vqResultFilter& filter)
{
  QTE_D(vqResultFilterDialog);

  if (filter.Threshold >= 0.0)
    {
    d->UI.threshold->setChecked(true);
    const int presetIndex = d->UI.thresholdPreset->findData(filter.Threshold);
    if (presetIndex >= 0)
      {
      d->UI.thresholdPreset->setCurrentIndex(presetIndex);
      }
    else
      {
      d->UI.thresholdPreset->setCurrentIndex(
        d->UI.thresholdPreset->count() - 1);
      d->UI.thresholdValue->setValue(filter.Threshold);
      }
    }
  else
    {
    d->UI.threshold->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
void vqResultFilterDialog::clearFilters()
{
  QTE_D(vqResultFilterDialog);

  d->UI.threshold->setChecked(false);
}

//-----------------------------------------------------------------------------
void vqResultFilterDialog::setThresholdFromPreset(int index)
{
  QTE_D(vqResultFilterDialog);

  const double value = d->UI.thresholdPreset->itemData(index).toDouble();
  if (value >= 0.0)
    {
    d->UI.thresholdValue->setValue(value);
    d->UI.thresholdValue->setReadOnly(true);
    }
  else
    {
    d->UI.thresholdValue->setReadOnly(false);
    }
}
