// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
