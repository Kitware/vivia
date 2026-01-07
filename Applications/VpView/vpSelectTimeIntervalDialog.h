// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
