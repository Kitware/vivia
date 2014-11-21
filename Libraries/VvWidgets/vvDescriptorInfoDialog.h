/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvDescriptorInfoDialog_h
#define __vvDescriptorInfoDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvDescriptor.h>

class vvDescriptorInfoDialogPrivate;

class VV_WIDGETS_EXPORT vvDescriptorInfoDialog : public QDialog
{
  Q_OBJECT

public:
  explicit vvDescriptorInfoDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~vvDescriptorInfoDialog();

  QList<vvDescriptor> descriptors() const;

public slots:
  void setDescriptors(QList<vvDescriptor>);
  void setDescriptors(std::vector<vvDescriptor>);
  void clearDescriptors();

protected slots:
  void toggleDetails(bool);
  void showDetails(QList<vvDescriptor>);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvDescriptorInfoDialog)

private:
  QTE_DECLARE_PRIVATE(vvDescriptorInfoDialog)
  Q_DISABLE_COPY(vvDescriptorInfoDialog)
};

#endif
