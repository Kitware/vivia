// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqRegionEditDialog.h"
#include "ui_regionEdit.h"

#include <QMessageBox>

#include <cmath>

#include <vgGeodesy.h>
#include <vgGeoTypes.h>

#include <vgGeoUtil.h>

QTE_IMPLEMENT_D_FUNC(vqRegionEditDialog)

static const char* MGRS_REGEXP =
  "([1-9]|[1-5][0-9]|60)[CDEFGHJKLMNPQRSTUVWX]" // Grid designation
  "([ABCDEFGHJKLMNPQRSTUVWXYZ][ABCDEFGHJKLMNPQRSTUV]" // Square designation
  "([0-9][0-9]){0,5})?"; // Refinement

//-----------------------------------------------------------------------------
class vqRegionEditDialogPrivate
{
public:
  double diameter() const;

  Ui::vqRegionEditDialog UI;
  vgGeocodedPoly Region;

  double LastDiameter;
};

//-----------------------------------------------------------------------------
double vqRegionEditDialogPrivate::diameter() const
{
  const int unitIndex = this->UI.diameterUnits->currentIndex();
  const double scale = this->UI.diameterUnits->itemData(unitIndex).toDouble();
  return this->UI.diameter->value() * scale;
}

//-----------------------------------------------------------------------------
vqRegionEditDialog::vqRegionEditDialog(QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags), d_ptr(new vqRegionEditDialogPrivate)
{
  QTE_D(vqRegionEditDialog);
  d->UI.setupUi(this);

  connect(d->UI.diameterPreset, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setDiameterPreset(int)));
  connect(d->UI.diameterUnits, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setDiameterUnits(int)));
  connect(d->UI.diameter, SIGNAL(valueChanged(double)),
          this, SLOT(updateDiameter()));

  d->UI.crs->addItem("WGS '84", vgGeodesy::LatLon_Wgs84);
  d->UI.crs->addItem("NAD '83", vgGeodesy::LatLon_Nad83);
  d->UI.crs->setCurrentIndex(0);

  d->UI.diameterPreset->addItem("Small (NFOV)", 10);
  d->UI.diameterPreset->addItem("Medium (MFOV)", 50);
  d->UI.diameterPreset->addItem("Large (WFOV)", 100);
  d->UI.diameterPreset->addItem("Custom", -1);

  d->UI.diameterUnits->addItem("m", 1.0);
  d->UI.diameterUnits->addItem("km", 1000.0);
  d->UI.diameterUnits->addItem("ft", 0.3048);
  d->UI.diameterUnits->addItem("yd", 0.9144);
  d->UI.diameterUnits->addItem("mi", 1609.344);

  d->UI.diameterPreset->setCurrentIndex(1);
  d->UI.diameterUnits->setCurrentIndex(0);

  this->updateDiameter();
}

//-----------------------------------------------------------------------------
vqRegionEditDialog::~vqRegionEditDialog()
{
}

//-----------------------------------------------------------------------------
vgGeocodedPoly vqRegionEditDialog::region() const
{
  QTE_D_CONST(vqRegionEditDialog);
  return d->Region;
}

//-----------------------------------------------------------------------------
void vqRegionEditDialog::setRegion(const vgGeocodedPoly& region)
{
  QTE_D(vqRegionEditDialog);

  // Get region CRS
  int crsIndex = d->UI.crs->findData(region.GCS);
  if (crsIndex >= 0)
    d->UI.crs->setCurrentIndex(crsIndex);

  // Get region center
  const QString loc =
    vgGeodesy::coordString(vgGeodesy::regionCenter(region));
  d->UI.location->setText(loc.left(loc.indexOf('(')));

  // Estimate region diameter
  // \TODO

  // Determine best-fit MGRS
  // \TODO
}

//-----------------------------------------------------------------------------
void vqRegionEditDialog::accept()
{
  QTE_D(vqRegionEditDialog);

  if (d->UI.mgrs->isChecked())
    {
    // Get the location in simplified form
    QString mgrs = d->UI.mgrsLocation->text().toUpper();
    mgrs.replace(QRegExp("\\s"), "");

    // Check that the location is valid
    if (!QRegExp(MGRS_REGEXP).exactMatch(mgrs))
      {
      const QString msg =
        "The MGRS location you have specified is not valid. Please try again.";
      QMessageBox::critical(this, "Error", msg);
      return;
      }

    // Generate the region
    d->Region = vgGeodesy::convertGcs(vgGeodesy::regionFromMgrs(mgrs),
                                      vgGeodesy::LatLon_Wgs84);
    }
  else
    {
    // Parse location center
    const int crs = d->UI.crs->itemData(d->UI.crs->currentIndex()).toInt();
    QString error;
    vgGeocodedCoordinate center =
      vgGeodesy::parseCoordinate(d->UI.location->text(), crs, &error);

    // Check for error
    if (center.GCS < 0)
      {
      QString msg =
        "The location you have specified is not valid. Please try again.";
      if (!error.isEmpty())
        msg += "\n\nAdditional information:\n" + error;
      QMessageBox::critical(this, "Error", msg);
      return;
      }

    // Generate the region
    const vgGeocodedPoly r = vgGeodesy::generateRegion(center, d->diameter());
    d->Region = vgGeodesy::convertGcs(r, crs);
    }

  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vqRegionEditDialog::setDiameterPreset(int presetIndex)
{
  QTE_D(vqRegionEditDialog);
  const int value = d->UI.diameterPreset->itemData(presetIndex).toInt();
  if (value < 0)
    {
    d->UI.diameter->setReadOnly(false);
    d->UI.diameterUnits->setEnabled(true);
    }
  else
    {
    d->UI.diameter->setReadOnly(true);
    d->UI.diameterUnits->setCurrentIndex(0);
    d->UI.diameterUnits->setEnabled(false);
    d->UI.diameter->setValue(value);
    }
}

//-----------------------------------------------------------------------------
void vqRegionEditDialog::setDiameterUnits(int unitIndex)
{
  QTE_D(vqRegionEditDialog);

  const double scale = d->UI.diameterUnits->itemData(unitIndex).toDouble();
  const double value = d->LastDiameter;

  const int decimals = scale >= 100.0 ? 2 : scale >= 10.0 ? 1 : 0;
  const double min = pow(10.0, -decimals);
  d->UI.diameter->setDecimals(decimals);
  d->UI.diameter->setRange(min, 10000.0 - min);
  d->UI.diameter->setSingleStep(decimals ? 0.1 : 1.0);

  d->UI.diameter->setValue(value / scale);
  this->updateDiameter();
}

//-----------------------------------------------------------------------------
void vqRegionEditDialog::updateDiameter()
{
  QTE_D(vqRegionEditDialog);
  d->LastDiameter = d->diameter();
}
