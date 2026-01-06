/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqDrawBoxQueryDialog_h
#define __vqDrawBoxQueryDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include <vvQueryFormulation.h>

class qtStatusManager;

class vqDrawBoxQueryDialogPrivate;

class vqDrawBoxQueryDialog : public QDialog
{
  Q_OBJECT

public:
  vqDrawBoxQueryDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vqDrawBoxQueryDialog();

  void initialize();

  std::string exemplarUri() const;
  std::vector<vvImageBoundingBox> drawnBoxes() const;

signals:
  void queryFormulationRequested(vvProcessingRequest,
                                 bool bypassCache,
                                 qtStatusManager* statusTarget);

public slots:
  virtual int exec();
  void finalizeBoxEdit();

protected slots:
  void chooseImage();
  void addBox();
  void removeSelectedBoxes();
  void clearAllBoxes();

  void cancelBoxEdit();
  void unsetBoxEditCursor();

  void updateBoxList();
  void updateButtonStates();

  void fitImageToView();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vqDrawBoxQueryDialog)

private:
  QTE_DECLARE_PRIVATE(vqDrawBoxQueryDialog)
};

#endif
