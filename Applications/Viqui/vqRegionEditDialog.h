// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
