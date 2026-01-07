// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgUserManualAction_h
#define __vgUserManualAction_h

#include <QAction>

#include <vgExport.h>

class QTVG_WIDGETS_EXPORT vgUserManualAction : public QAction
{
  Q_OBJECT

public:
  explicit vgUserManualAction(QObject* parent);
  virtual ~vgUserManualAction();

protected slots:
  void showUserManual();

private:
  Q_DISABLE_COPY(vgUserManualAction)
};

#endif
