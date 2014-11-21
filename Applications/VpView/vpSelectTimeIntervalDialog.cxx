/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpSelectTimeIntervalDialog.h"

#include "ui_vpSelectTimeIntervalDialog.h"

#include "vpUtils.h"
#include "vpViewCore.h"

//-----------------------------------------------------------------------------
class vpSelectTimeIntervalDialogPrivate
{
public:
  Ui_vpSelectTimeIntervalDialog UI;

  vpViewCore* ViewCore;

  vtkVgTimeStamp StartTime;
  vtkVgTimeStamp EndTime;

public:
  void updateTimeLabel(QLabel* label, const vtkVgTimeStamp& timeStamp)
    {
    if (timeStamp.IsValid())
      {
      int offset = this->ViewCore->getUseZeroBasedFrameNumbers() ? 0 : 1;
      label->setText(vpUtils::GetTimeAndFrameNumberString(timeStamp, offset));
      }
    else
      {
      label->setText("(none)");
      }
    }
};

QTE_IMPLEMENT_D_FUNC(vpSelectTimeIntervalDialog)

//-----------------------------------------------------------------------------
vpSelectTimeIntervalDialog::vpSelectTimeIntervalDialog(
  vpViewCore* coreInstance,
  QWidget* parent, Qt::WindowFlags flags) :
  QDialog(parent, flags), d_ptr(new vpSelectTimeIntervalDialogPrivate)
{
  QTE_D(vpSelectTimeIntervalDialog);

  d->UI.setupUi(this);
  d->ViewCore = coreInstance;

  connect(d->UI.startHere, SIGNAL(clicked()),
          this, SLOT(setStartTimeToCurrentFrame()));
  connect(d->UI.endHere, SIGNAL(clicked()),
          this, SLOT(setEndTimeToCurrentFrame()));

  this->update();
}

//-----------------------------------------------------------------------------
vpSelectTimeIntervalDialog::~vpSelectTimeIntervalDialog()
{
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpSelectTimeIntervalDialog::getStartTime() const
{
  QTE_D_CONST(vpSelectTimeIntervalDialog);
  return d->StartTime;
}

//-----------------------------------------------------------------------------
vtkVgTimeStamp vpSelectTimeIntervalDialog::getEndTime() const
{
  QTE_D_CONST(vpSelectTimeIntervalDialog);
  return d->EndTime;
}

//-----------------------------------------------------------------------------
double vpSelectTimeIntervalDialog::getDurationInSeconds() const
{
  QTE_D_CONST(vpSelectTimeIntervalDialog);
  return (d->StartTime.HasTime() && d->EndTime.HasTime())
           ? d->EndTime.GetTimeDifferenceInSecs(d->StartTime)
           : -1.0;
}

//-----------------------------------------------------------------------------
void vpSelectTimeIntervalDialog::reject()
{
  QTE_D(vpSelectTimeIntervalDialog);

  d->StartTime.Reset();
  d->EndTime.Reset();

  QDialog::reject();
}

//-----------------------------------------------------------------------------
void vpSelectTimeIntervalDialog::setStartTimeToCurrentFrame()
{
  QTE_D(vpSelectTimeIntervalDialog);

  d->StartTime = d->ViewCore->getCoreTimeStamp();

  if (d->EndTime.IsValid() && d->EndTime < d->StartTime)
    {
    d->EndTime = d->StartTime;
    }

  this->update();
}

//-----------------------------------------------------------------------------
void vpSelectTimeIntervalDialog::setEndTimeToCurrentFrame()
{
  QTE_D(vpSelectTimeIntervalDialog);

  d->EndTime = d->ViewCore->getCoreTimeStamp();

  if (d->StartTime.IsValid() && d->EndTime < d->StartTime)
    {
    d->StartTime = d->EndTime;
    }

  this->update();
}

//-----------------------------------------------------------------------------
void vpSelectTimeIntervalDialog::update()
{
  QTE_D(vpSelectTimeIntervalDialog);

  d->updateTimeLabel(d->UI.startTime, d->StartTime);
  d->updateTimeLabel(d->UI.endTime, d->EndTime);

  double duration = this->getDurationInSeconds();
  if (duration == -1.0)
    {
    d->UI.duration->setText("(invalid)");
    }
  else
    {
    d->UI.duration->setText(QString("%1 s").arg(duration));
    }
}
