/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpComputeColorRangeDialog_h
#define __vpComputeColorRangeDialog_h

#include <QDialog>

#include <qtGlobal.h>

//class vgAttributeSet;
//class vpTreeView;
//class vtkVgTrackModel;
//class vtkVgTrackTypeRegistry;

class vpComputeColorRangeDialogPrivate;
class vpViewCore;

class vpComputeColorRangeDialog : public QDialog
{
  Q_OBJECT

public:
  vpComputeColorRangeDialog(QWidget* parent, vpViewCore* core);
  ~vpComputeColorRangeDialog();

  double getComputedWidth() { return this->ComputedWidth; }
  double getComputedCenter() { return this->ComputedCenter; }

signals:
  void updateRequested(vpComputeColorRangeDialog* dlg);

protected slots:
  void computeFrameColorRange();
  void computeAllFramesColorRange();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpComputeColorRangeDialog)

private:
  QTE_DECLARE_PRIVATE(vpComputeColorRangeDialog)
    Q_DISABLE_COPY(vpComputeColorRangeDialog)

  double ComputedWidth;
  double ComputedCenter;
};

#endif
