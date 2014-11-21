/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqRegionEditDialog_h
#define __vqRegionEditDialog_h

#include <QDialog>

#include <qtGlobal.h>

struct vgGeocodedPoly;

class vqRegionEditDialogPrivate;

class vqRegionEditDialog : public QDialog
{
  Q_OBJECT

public:
  vqRegionEditDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
  virtual ~vqRegionEditDialog();

  vgGeocodedPoly region() const;
  void setRegion(const vgGeocodedPoly&);

  virtual void accept();

protected slots:
  void setDiameterPreset(int presetIndex);
  void setDiameterUnits(int unitIndex);
  void updateDiameter();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vqRegionEditDialog)

private:
  QTE_DECLARE_PRIVATE(vqRegionEditDialog)
  Q_DISABLE_COPY(vqRegionEditDialog)
};

#endif
