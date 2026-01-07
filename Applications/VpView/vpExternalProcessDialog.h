// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpExternalProcessDialog_h
#define __vpExternalProcessDialog_h

// QT includes.
#include <QDialog>

#include "ui_vpExternalProcessDialog.h"

// Forward declarations.
class vpSettings;

class vpExternalProcessDialog : public QDialog
{
  Q_OBJECT

public:
  vpExternalProcessDialog(QWidget* parent);
  ~vpExternalProcessDialog();

  Ui::vpExternalProcessDialog UI;

  QString getProgram() const;
  QStringList getArguments() const;
  QString getIOPath() const;

public slots:
  virtual void executeProcess();
  virtual void saveWithoutExecuting();
  virtual void cancel();

private:
  vpSettings* Settings;

  void save();
  void restore();
};

#endif // __vpExternalProcessDialog_h
