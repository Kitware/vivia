/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqClassifierQueryDialog.h"
#include "ui_classifierQuery.h"

#include <QCheckBox>
#include <QHash>
#include <QSettings>

#include <qtStlUtil.h>
#include <qtUtil.h>

#include <vvDescriptor.h>

#include "vqEventInfo.h"

QTE_IMPLEMENT_D_FUNC(vqClassifierQueryDialog)

//-----------------------------------------------------------------------------
class vqClassifierQueryDialogPrivate
{
public:
  void populateGroup(const QString& groupName, vqEventInfo::Groups typeGroups,
                     const QSet<int>& searchableTypes);

  void populateDescriptors();

  void addDescriptor(std::vector<vvDescriptor>& dest,
                     int type, const QString& moduleName,
                     const QString& descriptorName) const;
  void addDescriptors(std::vector<vvDescriptor>& dest, int type) const;

  struct DescriptorData
    {
    DescriptorData(const QString& moduleName, const QString& descriptorName)
      : ModuleName(moduleName), DescriptorName(descriptorName) {}

    QString ModuleName;
    QString DescriptorName;

    bool operator==(const DescriptorData& other) const
      {
      return this->ModuleName == other.ModuleName &&
             this->DescriptorName == other.DescriptorName;
      }
    };

  QHash<QCheckBox*, DescriptorData> DescriptorMap;
  QHash<DescriptorData, QCheckBox*> DescriptorWidgets;

  Ui::vqClassifierQueryDialog UI;
};

//-----------------------------------------------------------------------------
uint qHash(const vqClassifierQueryDialogPrivate::DescriptorData& dd)
{
  return qHash(dd.DescriptorName) ^ qHash(dd.ModuleName);
}

//-----------------------------------------------------------------------------
void vqClassifierQueryDialogPrivate::populateGroup(
  const QString& groupName, vqEventInfo::Groups typeGroups,
  const QSet<int>& searchableTypes)
{
  int group = this->UI.classifierFilter->beginGroup(groupName, true);
  foreach (const vqEventInfo& ei, vqEventInfo::types(typeGroups))
    {
    if (searchableTypes.contains(ei.Type))
      {
      this->UI.classifierFilter->addItem(ei.Type, ei.Name);
      this->UI.classifierFilter->setValue(ei.Type, 0.1);
      }
    }
  this->UI.classifierFilter->endGroup();
  this->UI.classifierFilter->setExpanded(group);
  this->UI.classifierFilter->setGroupState(group, false);
}

//-----------------------------------------------------------------------------
void vqClassifierQueryDialogPrivate::populateDescriptors()
{
  QVBoxLayout* layout = new QVBoxLayout();

  QSettings settings;
  int size = settings.beginReadArray("EventDescriptors");

  for (int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);

    QString displayName     = settings.value("DisplayName").toString();
    QString moduleName      = settings.value("ModuleName").toString();
    QString descriptorName  = settings.value("DescriptorName").toString();

    QCheckBox* item = new QCheckBox(displayName);
    const DescriptorData dd(moduleName, descriptorName);
    this->DescriptorMap.insert(item, dd);
    this->DescriptorWidgets.insert(dd, item);
    layout->addWidget(item);
    }

  settings.endArray();

  this->UI.eventDescriptorsWidget->setLayout(layout);
}

//-----------------------------------------------------------------------------
void vqClassifierQueryDialogPrivate::addDescriptor(
  std::vector<vvDescriptor>& dest, int type, const QString& moduleName,
  const QString& descriptorName) const
{
  vvDescriptor desc;

  desc.DescriptorName = stdString(descriptorName);
  desc.Values.clear();

  int minSize = qMax(vqEventInfo::staticTypeArraySize(), type + 1);
  desc.Values.push_back(std::vector<float>());
  desc.Values[0].assign(minSize, -1.0);

  desc.Values[0][type] = this->UI.classifierFilter->value(type);

  desc.ModuleName = stdString(moduleName);
  dest.push_back(desc);
}

//-----------------------------------------------------------------------------
void vqClassifierQueryDialogPrivate::addDescriptors(
  std::vector<vvDescriptor>& dest, int type) const
{
  typedef QHash<QCheckBox*, DescriptorData>::const_iterator Iterator;
  foreach_iter (Iterator, iter, this->DescriptorMap)
    {
    if (iter.key()->isChecked())
      {
      this->addDescriptor(dest, type, iter.value().ModuleName,
                          iter.value().DescriptorName);
      }
    }
}

//-----------------------------------------------------------------------------
vqClassifierQueryDialog::vqClassifierQueryDialog(QWidget* parent,
                                                 Qt::WindowFlags flags)
  : vvAbstractSimilarityQueryDialog(parent, flags),
    d_ptr(new vqClassifierQueryDialogPrivate)
{
  QTE_D(vqClassifierQueryDialog);

  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  d->populateDescriptors();

  QSet<int> searchableTypes = vqEventInfo::searchableTypes();
  d->populateGroup("Fish", vqEventInfo::Fish, searchableTypes);
  d->populateGroup("Scallop", vqEventInfo::Scallop, searchableTypes);
}

//-----------------------------------------------------------------------------
vqClassifierQueryDialog::~vqClassifierQueryDialog()
{
}

//-----------------------------------------------------------------------------
std::vector<vvDescriptor> vqClassifierQueryDialog::selectedDescriptors() const
{
  QTE_D_CONST(vqClassifierQueryDialog);

  std::vector<vvDescriptor> result;

  foreach (int type, d->UI.classifierFilter->keys())
    {
    if (d->UI.classifierFilter->state(type))
      {
      d->addDescriptors(result, type);
      }
    }

  return result;
}

//-----------------------------------------------------------------------------
void vqClassifierQueryDialog::setSelectedDescriptors(
  const std::vector<vvDescriptor>& descriptors)
{
  QTE_D(vqClassifierQueryDialog);

  size_t n = descriptors.size();
  while (n--)
    {
    const vvDescriptor& di = descriptors[n];
    if (di.Values.size() == 1)
      {
      vqClassifierQueryDialogPrivate::DescriptorData dd(
        qtString(di.ModuleName), qtString(di.DescriptorName));
      if (d->DescriptorWidgets.contains(dd))
        {
        d->DescriptorWidgets[dd]->setChecked(true);
        }

      size_t j = di.Values[0].size();
      while (j--)
        {
        double p = di.Values[0][j];
        if (p >= 0.0)
          {
          d->UI.classifierFilter->setState(static_cast<int>(j), true);
          d->UI.classifierFilter->setValue(static_cast<int>(j), p);
          }
        }
      }
    }
}
