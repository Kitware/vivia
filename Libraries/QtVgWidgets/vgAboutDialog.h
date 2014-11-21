/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
