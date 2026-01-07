// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgAboutDialog_h
#define __vgAboutDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include <vgExport.h>

class QAction;

class vgAboutDialogPrivate;

class QTVG_WIDGETS_EXPORT vgAboutDialog : public QDialog
{
  Q_OBJECT

public:
  explicit vgAboutDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~vgAboutDialog();

  int exec();

protected slots:
  void showContextMenu(QPoint);
  void copyToClipboard();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgAboutDialog)

private:
  QTE_DECLARE_PRIVATE(vgAboutDialog)
  Q_DISABLE_COPY(vgAboutDialog)
};

#endif
