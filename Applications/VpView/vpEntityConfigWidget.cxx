// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpEntityConfigWidget.h"

#include "vpEntityConfig.h"
#include "vgEntityType.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include <vector>

class vpEntityConfigWidget::vgInternal
{
public:
  QGridLayout* Layout;

  struct RowWidgets;
  std::vector<RowWidgets> Rows;

  QCheckBox* ShowAll;

  vpEntityConfig* EntityConfig;
};

struct vpEntityConfigWidget::vgInternal::RowWidgets
{
  QLabel* Name;
  QComboBox* RenderStyle;
  QCheckBox* HasSecondaryColor;
  QPushButton* PrimaryColor;
  QPushButton* SecondaryColor;
};

//-----------------------------------------------------------------------------
vpEntityConfigWidget::vpEntityConfigWidget(QWidget* p)
  : QWidget(p)
{
  this->Internal = new vgInternal;

  this->Internal->Layout = new QGridLayout;
  this->Internal->Layout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(this->Internal->Layout);

  this->Internal->ShowAll = new QCheckBox;
  this->Internal->ShowAll->setText("Show All");
  connect(this->Internal->ShowAll, SIGNAL(stateChanged(int)),
          SLOT(ShowAllTypes(int)));

  this->Internal->Layout->addWidget(this->Internal->ShowAll, 0, 0);
}

//-----------------------------------------------------------------------------
vpEntityConfigWidget::~vpEntityConfigWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::Initialize(vpEntityConfig* config)
{
  this->Internal->EntityConfig = config;

  for (int i = 0, end = config->GetNumberOfTypes(); i < end; ++i)
    {
    const vgEntityType& at = config->GetEntityType(i);
    this->AddType(at);
    this->UpdateRow(i);
    }

  this->Update();
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::AddType(const vgEntityType& type)
{
  vgInternal::RowWidgets widgets;

  widgets.Name = new QLabel(type.GetName());

  widgets.RenderStyle = new QComboBox;
  widgets.RenderStyle->addItem("Random Colors");
  widgets.RenderStyle->addItem("Role-Based Colors");
  widgets.RenderStyle->setToolTip("Render Style");

  widgets.PrimaryColor = new QPushButton;
  widgets.PrimaryColor->setToolTip("Primary Color");

  widgets.SecondaryColor = new QPushButton;
  widgets.SecondaryColor->setToolTip("Secondary Color");

  widgets.HasSecondaryColor = new QCheckBox;
  widgets.HasSecondaryColor->setToolTip("Use Secondary Color");

  int row = 1 + static_cast<int>(this->Internal->Rows.size());
  this->Internal->Layout->addWidget(widgets.Name, row, 0);
  this->Internal->Layout->addWidget(widgets.RenderStyle, row, 2);
  this->Internal->Layout->addWidget(widgets.PrimaryColor, row, 3);
  this->Internal->Layout->addWidget(widgets.HasSecondaryColor, row, 4);
  this->Internal->Layout->addWidget(widgets.SecondaryColor, row, 5);

  this->Internal->Layout->setColumnStretch(1, 1);

  connect(widgets.RenderStyle, SIGNAL(currentIndexChanged(int)),
          SLOT(RenderStyleChanged(int)));

  connect(widgets.HasSecondaryColor, SIGNAL(stateChanged(int)),
          SLOT(HasSecondaryColorChanged(int)));

  connect(widgets.PrimaryColor, SIGNAL(clicked()),
          SLOT(PrimaryColorClicked()));

  connect(widgets.SecondaryColor, SIGNAL(clicked()),
          SLOT(SecondaryColorClicked()));

  this->Internal->Rows.push_back(widgets);

  for (int i = 0, end = this->Internal->Layout->columnCount(); i < end; ++i)
    {
    if (QLayoutItem* item = this->Internal->Layout->itemAtPosition(row, i))
      {
      item->widget()->setProperty("Type Index", row - 1);
      }
    }
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::UpdateRow(int row)
{
  const vgInternal::RowWidgets& widgets = this->Internal->Rows[row];
  const vgEntityType& type = this->GetType(row);

  if (type.GetUseRandomColors())
    {
    widgets.PrimaryColor->setDisabled(true);
    widgets.PrimaryColor->setStyleSheet(QString());
    widgets.HasSecondaryColor->setDisabled(true);
    widgets.SecondaryColor->setDisabled(true);
    widgets.SecondaryColor->setStyleSheet(QString());
    widgets.RenderStyle->setCurrentIndex(0);
    }
  else
    {
    widgets.PrimaryColor->setDisabled(false);
    widgets.HasSecondaryColor->setDisabled(false);
    widgets.RenderStyle->setCurrentIndex(1);

    const char* colorStyle = "QPushButton { background-color : %1 }";

    QColor color = this->GetPrimaryColor(type);
    widgets.PrimaryColor->setStyleSheet(QString(colorStyle).arg(color.name()));

    if (type.GetHasSecondaryColor())
      {
      widgets.SecondaryColor->setDisabled(false);
      widgets.HasSecondaryColor->setCheckState(Qt::Checked);

      QColor color = this->GetSecondaryColor(type);
      widgets.SecondaryColor->setStyleSheet(QString(colorStyle).arg(color.name()));
      }
    else
      {
      widgets.SecondaryColor->setStyleSheet(QString());
      widgets.SecondaryColor->setDisabled(true);
      }
    }
}

//-----------------------------------------------------------------------------
int vpEntityConfigWidget::GetWidgetRow(QObject* obj)
{
  return obj->property("Type Index").toInt();
}

//-----------------------------------------------------------------------------
const vgEntityType& vpEntityConfigWidget::GetType(int row)
{
  return this->Internal->EntityConfig->GetEntityType(row);
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::RenderStyleChanged(int index)
{
  int row = this->GetWidgetRow(this->sender());
  vgEntityType at = this->GetType(row);

  at.SetUseRandomColors(index == 0);

  this->UpdateEntityType(row, at);
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::HasSecondaryColorChanged(int state)
{
  int row = this->GetWidgetRow(this->sender());
  vgEntityType at = this->GetType(row);

  at.SetHasSecondaryColor(state == Qt::Checked);

  this->UpdateEntityType(row, at);
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::PrimaryColorClicked()
{
  int row = this->GetWidgetRow(this->sender());
  vgEntityType at = this->GetType(row);

  QColor color = QColorDialog::getColor(this->GetPrimaryColor(at), this);
  if (!color.isValid())
    {
    return;
    }
  at.SetColor(color.redF(), color.greenF(), color.blueF());

  this->UpdateEntityType(row, at);
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::SecondaryColorClicked()
{
  int row = this->GetWidgetRow(this->sender());
  vgEntityType at = this->GetType(row);

  QColor color = QColorDialog::getColor(this->GetSecondaryColor(at), this);
  if (!color.isValid())
    {
    return;
    }
  at.SetSecondaryColor(color.redF(), color.greenF(), color.blueF());

  this->UpdateEntityType(row, at);
}

//-----------------------------------------------------------------------------
QColor vpEntityConfigWidget::GetPrimaryColor(const vgEntityType& type)
{
  double r, g, b;
  type.GetColor(r, g, b);
  QColor color;
  color.setRgbF(r, g, b);
  return color;
}

//-----------------------------------------------------------------------------
QColor vpEntityConfigWidget::GetSecondaryColor(const vgEntityType& type)
{
  double r, g, b;
  type.GetSecondaryColor(r, g, b);
  QColor color;
  color.setRgbF(r, g, b);
  return color;
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::UpdateEntityType(int row,
                                            const vgEntityType& type)
{
  this->Internal->EntityConfig->SetEntityType(row, type);
  this->UpdateRow(row);
  emit this->EntityTypeChanged(row);
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::ShowAllTypes(int state)
{
  for (size_t i = 0, end = this->Internal->Rows.size(); i < end; ++i)
    {
    bool show = this->GetType(static_cast<int>(i)).GetIsUsed() ||
                state == Qt::Checked;

    this->Internal->Rows[i].Name->setVisible(show);
    this->Internal->Rows[i].RenderStyle->setVisible(show);
    this->Internal->Rows[i].HasSecondaryColor->setVisible(show);
    this->Internal->Rows[i].PrimaryColor->setVisible(show);
    this->Internal->Rows[i].SecondaryColor->setVisible(show);
    }
}

//-----------------------------------------------------------------------------
void vpEntityConfigWidget::Update()
{
  this->ShowAllTypes(this->Internal->ShowAll->checkState());
}
