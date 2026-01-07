// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
