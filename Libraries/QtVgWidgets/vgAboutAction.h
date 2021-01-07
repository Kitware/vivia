// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgAboutAction_h
#define __vgAboutAction_h

#include <QAction>

#include <vgExport.h>

class QTVG_WIDGETS_EXPORT vgAboutAction : public QAction
{
  Q_OBJECT

public:
  explicit vgAboutAction(QObject* parent);
  virtual ~vgAboutAction();

protected slots:
  void showDialog();

private:
  Q_DISABLE_COPY(vgAboutAction)
};

#endif
