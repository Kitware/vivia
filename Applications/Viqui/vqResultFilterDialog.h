/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqResultFilterDialog_h
#define __vqResultFilterDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include "vqResultFilter.h"

class vqResultFilterDialogPrivate;

class vqResultFilterDialog : public QDialog
{
  Q_OBJECT

public:
  vqResultFilterDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
  virtual ~vqResultFilterDialog();

  vqResultFilter filter() const;
  void setFilter(const vqResultFilter&);

protected slots:
  void clearFilters();

  void setThresholdFromPreset(int);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vqResultFilterDialog)

private:
  QTE_DECLARE_PRIVATE(vqResultFilterDialog)
};

#endif
