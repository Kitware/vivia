/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
