/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsVvqsDatabaseQueryDialog_h
#define __vsVvqsDatabaseQueryDialog_h

#include <QDialog>

#include <qtGlobal.h>

class QDateTime;
class QUrl;

class vsVvqsDatabaseQueryDialogPrivate;

class vsVvqsDatabaseQueryDialog : public QDialog
{
  Q_OBJECT

public:
  vsVvqsDatabaseQueryDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vsVvqsDatabaseQueryDialog();

  QUrl uri() const;

  virtual void accept();

protected slots:
  void updateTimeUpperFromLower();
  void updateTimeLowerFromUpper();

  void editServerUri();

  void validate();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsVvqsDatabaseQueryDialog)

private:
  QTE_DECLARE_PRIVATE(vsVvqsDatabaseQueryDialog)
  QTE_DISABLE_COPY(vsVvqsDatabaseQueryDialog)
};

#endif
