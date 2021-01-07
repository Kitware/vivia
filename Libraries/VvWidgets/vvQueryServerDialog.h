// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvQueryServerDialog_h
#define __vvQueryServerDialog_h

#include <QDialog>
#include <QUrl>

#include <qtGlobal.h>

#include <vgExport.h>

class vvAbstractQueryServerChooser;

class vvQueryServerDialogPrivate;

class VV_WIDGETS_EXPORT vvQueryServerDialog : public QDialog
{
  Q_OBJECT

public:
  vvQueryServerDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~vvQueryServerDialog();

  QUrl uri() const;

  void registerServerType(const QString& displayName,
                          const QRegExp& supportedSchemes,
                          vvAbstractQueryServerChooser* chooser);

  int exec(const QUrl& initialUri);

protected slots:
  void serverTypeChanged(int);
  void setUri(QUrl);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvQueryServerDialog)

private:
  QTE_DECLARE_PRIVATE(vvQueryServerDialog)
  Q_DISABLE_COPY(vvQueryServerDialog)

  using QDialog::exec;
};

#endif
