/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
