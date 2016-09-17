/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpTrackColorDialog.h"
#include "ui_vpTrackColorDialog.h"
#include "ui_vpTrackAttributeColorDialog.h"

#include <vtkVgTrack.h>

#include <vgAttributeSet.h>
#include <vgColor.h>

#include <QSettings>

QTE_IMPLEMENT_D_FUNC(vpTrackColorDialog)

namespace
{

// these should match the group/attribute strings set in vpViewCore
const QString fgSsd           = "ForegroundTracking/SSD";
const QString fgCsurf         = "ForegroundTracking/CSURF";
const QString daKinematic     = "DataAssociationTracking/Kinematic";
const QString daMultiFeatures = "DataAssociationTracking/MultiFeatures";
const QString daMultiple      = "DataAssociationTracking/Multiple";
const QString kfExtended      = "KalmanFilter/Extended";
const QString kfLinear        = "KalmanFilter/Linear";
const QString intForward      = "Interval/Forward";
const QString intBack         = "Interval/Back";
const QString intInit         = "Interval/Init";
const QString linkStart       = "Linking/Start";
const QString linkEnd         = "Linking/End";
const QString linkMultiple    = "Linking/Multiple";

}

//-----------------------------------------------------------------------------
class vpTrackColorDialogPrivate
{
public:
  Ui::vpTrackColorDialog UI;
  Ui::vpTrackAttributeColorDialog* AttributeUI;
};

//-----------------------------------------------------------------------------
vpTrackColorDialog::vpTrackColorDialog(const vgAttributeSet* trackAttributes,
                                       QWidget* parent)
  : QDialog(parent), d_ptr(new vpTrackColorDialogPrivate)
{
  QTE_D(vpTrackColorDialog);

  d->AttributeUI = 0;
  d->UI.setupUi(this);

  if (trackAttributes)
    {
    std::vector<std::string> groups = trackAttributes->GetEnabledGroups();
    if (!groups.empty())
      {
      for (size_t i = 0, size = groups.size(); i < size; ++i)
        {
        d->UI.attributeGroup->addItem(groups[i].c_str());
        }
      }
    }

  d->UI.colorByStateAttribute->setEnabled(d->UI.attributeGroup->count() != 0);
  d->UI.colorByStatePVO->setEnabled(true);

  // TODO: Use the real colors
  d->UI.fishColor->setColor(Qt::blue);
  d->UI.scallopColor->setColor(Qt::green);
  d->UI.otherColor->setColor(Qt::red);

  connect(d->UI.colorByStateAttribute, SIGNAL(toggled(bool)),
          d->UI.attributeGroup, SLOT(setEnabled(bool)));
  connect(d->UI.setColors, SIGNAL(clicked()), this, SLOT(setAttrColors()));
  connect(d->UI.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
          this, SLOT(apply()));
}

//-----------------------------------------------------------------------------
vpTrackColorDialog::~vpTrackColorDialog()
{
}

//-----------------------------------------------------------------------------
vpTrackColorDialog::ColoringMode vpTrackColorDialog::mode() const
{
  QTE_D_CONST(vpTrackColorDialog);

  return d->UI.colorByTypeLabel->isChecked() ? ColorByTypeLabel :
         d->UI.colorByStatePVO->isChecked() ? ColorByStatePVO :
         d->UI.colorByStateAttribute->isChecked() ? ColorByStateAttribute :
         d->UI.randomColoring->isChecked() ? RandomColor
                                                  : SingleColor;
}

//-----------------------------------------------------------------------------
void vpTrackColorDialog::setMode(vpTrackColorDialog::ColoringMode mode)
{
  QTE_D(vpTrackColorDialog);

  switch (mode)
    {
    case ColorByTypeLabel:
      d->UI.colorByTypeLabel->setChecked(true);
      break;
    case ColorByStatePVO:
      d->UI.colorByStatePVO->setChecked(true);
      break;
    case ColorByStateAttribute:
      d->UI.colorByStateAttribute->setChecked(true);
      break;
    case RandomColor:
      d->UI.randomColoring->setChecked(true);
      break;
    case SingleColor:
    default:
      d->UI.defaultColoring->setChecked(true);
    }
}

//-----------------------------------------------------------------------------
QString vpTrackColorDialog::attributeGroup() const
{
  QTE_D_CONST(vpTrackColorDialog);

  return d->UI.attributeGroup->itemText(d->UI.attributeGroup->currentIndex());
}

//-----------------------------------------------------------------------------
void vpTrackColorDialog::setAttributeGroup(const QString& attributeGroup)
{
  QTE_D_CONST(vpTrackColorDialog);

  for (int i = 0, count = d->UI.attributeGroup->count(); i < count; ++i)
    {
    if (d->UI.attributeGroup->itemText(i) == attributeGroup)
      {
      d->UI.attributeGroup->setCurrentIndex(i);
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void vpTrackColorDialog::accept()
{
  // TODO: save colors to settings here

  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vpTrackColorDialog::setAttrColors()
{
  QTE_D(vpTrackColorDialog);

  QDialog attrColorDialog;

  Ui::vpTrackAttributeColorDialog ui;
  ui.setupUi(&attrColorDialog);

  connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
          this, SLOT(applyAttributeColors()));

  QSettings settings;
  settings.beginGroup("TrackAttributeColors");

  const vgColor white(Qt::white);

  // read colors from settings
  ui.fgSsdColor->setColor(
    vgColor::read(settings, fgSsd, white).toQColor());
  ui.fgCsurfColor->setColor(
    vgColor::read(settings, fgCsurf, white).toQColor());

  ui.daKinematicColor->setColor(
    vgColor::read(settings, daKinematic, white).toQColor());
  ui.daMultiFeaturesColor->setColor(
    vgColor::read(settings, daMultiFeatures, white).toQColor());
  ui.daCombinedColor->setColor(
    vgColor::read(settings, daMultiple, white).toQColor());

  ui.kfExtendedColor->setColor(
    vgColor::read(settings, kfExtended, white).toQColor());
  ui.kfLinearColor->setColor(
    vgColor::read(settings, kfLinear, white).toQColor());

  ui.intForwardColor->setColor(
    vgColor::read(settings, intForward, white).toQColor());
  ui.intBackColor->setColor(
    vgColor::read(settings, intBack, white).toQColor());
  ui.intInitColor->setColor(
    vgColor::read(settings, intInit, white).toQColor());

  ui.linkStartColor->setColor(
    vgColor::read(settings, linkStart, white).toQColor());
  ui.linkEndColor->setColor(
    vgColor::read(settings, linkEnd, white).toQColor());
  ui.linkCombinedColor->setColor(
    vgColor::read(settings, linkMultiple, white).toQColor());

  // show the color selection dialog
  d->AttributeUI = &ui;
  if (attrColorDialog.exec() == QDialog::Accepted)
    {
    this->applyAttributeColors();
    }
  d->AttributeUI = 0;
}

//-----------------------------------------------------------------------------
void vpTrackColorDialog::apply()
{
  // TODO: save colors to settings here
  emit this->updateRequested(this);
}

//-----------------------------------------------------------------------------
void vpTrackColorDialog::applyAttributeColors()
{
  QTE_D(vpTrackColorDialog);

  Ui::vpTrackAttributeColorDialog* ui = d->AttributeUI;
  if (!ui)
    {
    return;
    }

  QSettings settings;
  settings.beginGroup("TrackAttributeColors");

  // write colors to settings
  vgColor::write(settings, fgSsd, ui->fgSsdColor->color());
  vgColor::write(settings, fgCsurf, ui->fgCsurfColor->color());
  vgColor::write(settings, daKinematic, ui->daKinematicColor->color());
  vgColor::write(settings, daMultiFeatures, ui->daMultiFeaturesColor->color());
  vgColor::write(settings, daMultiple, ui->daCombinedColor->color());
  vgColor::write(settings, kfExtended, ui->kfExtendedColor->color());
  vgColor::write(settings, kfLinear, ui->kfLinearColor->color());
  vgColor::write(settings, intForward, ui->intForwardColor->color());
  vgColor::write(settings, intBack, ui->intBackColor->color());
  vgColor::write(settings, intInit, ui->intInitColor->color());
  vgColor::write(settings, linkStart, ui->linkStartColor->color());
  vgColor::write(settings, linkEnd, ui->linkEndColor->color());
  vgColor::write(settings, linkMultiple, ui->linkCombinedColor->color());

  emit this->updateRequested(this);
}
