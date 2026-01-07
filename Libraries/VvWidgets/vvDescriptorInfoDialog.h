// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
