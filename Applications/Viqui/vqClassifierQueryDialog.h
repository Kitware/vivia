// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqClassifierQueryDialog_h
#define __vqClassifierQueryDialog_h

#include <qtGlobal.h>

#include <vvAbstractSimilarityQueryDialog.h>

struct vvDescriptor;

class vqClassifierQueryDialogPrivate;

class vqClassifierQueryDialog : public vvAbstractSimilarityQueryDialog
{
  Q_OBJECT

public:
  vqClassifierQueryDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vqClassifierQueryDialog();

  virtual std::vector<vvDescriptor> selectedDescriptors() const;
  virtual void setSelectedDescriptors(const std::vector<vvDescriptor>&);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vqClassifierQueryDialog)

private:
  QTE_DECLARE_PRIVATE(vqClassifierQueryDialog)
  Q_DISABLE_COPY(vqClassifierQueryDialog)
};

#endif
