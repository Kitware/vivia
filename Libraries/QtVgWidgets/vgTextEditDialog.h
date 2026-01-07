// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgTextEditDialog_h
#define __vgTextEditDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include <vgExport.h>

class vgTextEditDialogPrivate;

class QTVG_WIDGETS_EXPORT vgTextEditDialog : public QDialog
{
  Q_OBJECT

public:
  explicit vgTextEditDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~vgTextEditDialog();

  void setText(const QString& text);
  QString text() const;

  static QString getText(QWidget* parent,
                         const QString& title,
                         const QString& text = QString(),
                         bool* ok = 0, Qt::WindowFlags flags = 0);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgTextEditDialog)

private:
  QTE_DECLARE_PRIVATE(vgTextEditDialog)
  Q_DISABLE_COPY(vgTextEditDialog)
};

#endif
