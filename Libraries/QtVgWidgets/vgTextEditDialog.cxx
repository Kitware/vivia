// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgTextEditDialog.h"
#include "ui_vgTextEditDialog.h"

#include <qtUtil.h>

QTE_IMPLEMENT_D_FUNC(vgTextEditDialog)

//-----------------------------------------------------------------------------
class vgTextEditDialogPrivate
{
public:
  Ui::vgTextEditDialog UI;
};

//-----------------------------------------------------------------------------
vgTextEditDialog::vgTextEditDialog(QWidget* parent, Qt::WindowFlags f) :
  QDialog(parent, f),
  d_ptr(new vgTextEditDialogPrivate)
{
  QTE_D(vgTextEditDialog);

  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);
}

//-----------------------------------------------------------------------------
vgTextEditDialog::~vgTextEditDialog()
{
}

//-----------------------------------------------------------------------------
void vgTextEditDialog::setText(const QString& text)
{
  QTE_D(vgTextEditDialog);

  d->UI.textEdit->setPlainText(text);
  d->UI.textEdit->selectAll();
}

//-----------------------------------------------------------------------------
QString vgTextEditDialog::text() const
{
  QTE_D_CONST(vgTextEditDialog);

  return d->UI.textEdit->toPlainText();
}

//-----------------------------------------------------------------------------
QString vgTextEditDialog::getText(QWidget* parent,
                                  const QString& title,
                                  const QString& text,
                                  bool* ok, Qt::WindowFlags flags)
{
  vgTextEditDialog dialog(parent, flags);
  dialog.setWindowTitle(title);
  dialog.setText(text);

  int result = dialog.exec();
  if (ok)
    {
    *ok = result == QDialog::Accepted;
    }

  return dialog.text();
}
