/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpSelectTimeIntervalDialog_h
#define __vpSelectTimeIntervalDialog_h

#include <QDialog>

#include <qtGlobal.h>

class vpSelectTimeIntervalDialogPrivate;
class vpViewCore;

class vtkVgTimeStamp;

class vpSelectTimeIntervalDialog : public QDialog
{
  Q_OBJECT

public:
  vpSelectTimeIntervalDialog(vpViewCore* coreInstance,
                             QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vpSelectTimeIntervalDialog();

  vtkVgTimeStamp getStartTime() const;
  vtkVgTimeStamp getEndTime() const;

  double getDurationInSeconds() const;

  virtual void reject();

protected slots:
  void setStartTimeToCurrentFrame();
  void setEndTimeToCurrentFrame();

protected:
  void update();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpSelectTimeIntervalDialog)

private:
  QTE_DECLARE_PRIVATE(vpSelectTimeIntervalDialog)
};

#endif // __vpSelectTimeIntervalDialog_h
