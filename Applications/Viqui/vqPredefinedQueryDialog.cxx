/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqPredefinedQueryDialog.h"
#include "ui_predefinedQuery.h"

#include "vqEventInfo.h"
#include "vqPredefinedQueryCache.h"
#include "vqSettings.h"

#include <qtMap.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <QButtonGroup>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QStringList>
#include <QStyle>
#include <QVBoxLayout>

QTE_IMPLEMENT_D_FUNC(vqPredefinedQueryDialog)

//-----------------------------------------------------------------------------
static inline bool operator==(const vvDescriptor& a, const vvDescriptor& b)
{
  return vvDescriptor::compare(a, b, vvDescriptor::CompareDetection);
}

//-----------------------------------------------------------------------------
class vqPredefinedQueryDialogPrivate
{
public:
  vqPredefinedQueryDialogPrivate(vqPredefinedQueryDialog* q) : q_ptr(q) {}

  bool loadQueries();
  bool setupClassifierGroup();
  void addClassifierDescriptors(std::vector<vvDescriptor>& dest,
                                int type, double value) const;

  Ui::vqPredefinedQueryDialog UI;
  QButtonGroup* Buttons;

  struct PredefinedClassifierInfo
    {
    QStringList ModuleNames;
    QString DescriptorName;
    };

  QHash<int, vvQueryInstance> Queries;
  QHash<int, PredefinedClassifierInfo> PredefinedClassifiers;

  vvQueryInstance CurrentQuery;
  QString LastError;

protected:
  QTE_DECLARE_PUBLIC_PTR(vqPredefinedQueryDialog)


  struct Group
    {
    Group() {}
    Group(const QString& name) : Name(name) {}

    QString Name;
    QList<QRadioButton*> Buttons;
    };

  int populateGroup(QSettings& settings, const QString& groupName);
  void addClassifierDescriptor(std::vector<vvDescriptor>& dest,
                               int type, const QString& moduleName,
                               const QString& descriptorName,
                               double value) const;

private:
  QTE_DECLARE_PUBLIC(vqPredefinedQueryDialog)
};

//-----------------------------------------------------------------------------
bool vqPredefinedQueryDialogPrivate::loadQueries()
{
  QString path = vqSettings().predefinedQueryUri().toLocalFile();
  if (path.isEmpty())
    {
    this->LastError = "Location for predefined queries has not been set.";
    return false;
    }

  // Get predefined query plans from cache
  vqPredefinedQueryList plans =
    vqPredefinedQueryCache::getAvailableQueryPlans();
  if (plans.isEmpty())
    {
    this->LastError = "No valid query plans found in configured location.";
    return false;
    }

  QList<Group> groups;
  groups.append(Group("Ungrouped"));
  Group* ungroupedGroup = &groups.first();
  Group* lastGroup = 0;
  int currentId = 0;

  // Get sorted query names
  QStringList queryNames = qtUtil::localeSort(plans.keys());

  // Loop over queries
  foreach (QString queryName, queryNames)
    {
    // Get plan now, before we start monkeying with the name
    vvQueryInstance plan = plans[queryName];

    // Extract group for item, treating '.' as a group separator (i.e. file
    // names look like 'group.child.vqp')
    Group* group = 0;
    int n = queryName.indexOf('.');
    if (n < 0)
      {
      // Query has no group
      group = ungroupedGroup;
      }
    else
      {
      // Split name into group name and query name
      QString groupName = queryName.left(n);
      queryName = queryName.mid(n + 1);

      // Check if this belongs to the current group
      if (!(lastGroup && groupName == lastGroup->Name))
        {
        // No; must make new group
        groups.append(Group(groupName));
        lastGroup = &groups.last();
        }
      group = lastGroup;
      }

    // Create radio button for this query
    QRadioButton* button = new QRadioButton(this->UI.buttonBox);
    button->setText(queryName);
    group->Buttons.append(button);
    this->Buttons->addButton(button, currentId);

    // Add query to map
    this->Queries.insert(currentId, plan);
    ++currentId;
    }

  // Create group boxes for each group
  QVBoxLayout* topLayout = new QVBoxLayout();
  topLayout->setMargin(0);
  foreach (Group group, groups)
    {
    if (!group.Buttons.isEmpty())
      {
      QGroupBox* groupBox = new QGroupBox();
      QVBoxLayout* groupLayout = new QVBoxLayout();
      groupBox->setLayout(groupLayout);
      groupBox->setTitle(group.Name);

      qtUtil::mapBound(group.Buttons, groupLayout,
                       &QVBoxLayout::addWidget, 0, Qt::Alignment(0));
      topLayout->addWidget(groupBox);
      }
    }
  this->UI.groupsWidget->setLayout(topLayout);

  // Success
  return true;
}

//-----------------------------------------------------------------------------
bool vqPredefinedQueryDialogPrivate::setupClassifierGroup()
{
  QSettings settings;
  settings.beginGroup("PredefinedClassifiers");
  int count = this->populateGroup(settings, "Person");
  count += this->populateGroup(settings, "Vehicle");
  settings.endGroup();

  if (count == 0)
    {
    this->UI.classifierGroup->setVisible(false);
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
int vqPredefinedQueryDialogPrivate::populateGroup(QSettings& settings,
                                                  const QString& groupName)
{
  settings.beginGroup(groupName);

  int numAdded = 0;
  int group = this->UI.classifierFilter->beginGroup(groupName, true);
  QStringList childGroupList = settings.childGroups();
  foreach (QString childGroup, childGroupList)
    {
    settings.beginGroup(childGroup);
    if (settings.contains("DisplayName") &&
        settings.contains("ClassifierIndex") &&
        settings.contains("DescriptorName") &&
        settings.contains("ModuleName"))
      {
      int type = settings.value("ClassifierIndex").toInt();
      this->UI.classifierFilter->addItem(type,
                                         settings.value("DisplayName").toString());
      this->UI.classifierFilter->setValue(type, 0.1);
      PredefinedClassifierInfo info;
      info.DescriptorName = settings.value("DescriptorName").toString();
      info.ModuleNames = settings.value("ModuleName").toStringList();
      this->PredefinedClassifiers.insert(type, info);
      numAdded++;
      }
    settings.endGroup();
    }
  this->UI.classifierFilter->endGroup();
  this->UI.classifierFilter->setExpanded(group);
  this->UI.classifierFilter->setGroupState(group, false);

  settings.endGroup();

  return numAdded;
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryDialogPrivate::addClassifierDescriptors(
  std::vector<vvDescriptor>& dest, int type, double value) const
{
  QHash<int, PredefinedClassifierInfo>::const_iterator infoIter =
    this->PredefinedClassifiers.find(type);
  foreach (QString moduleName, infoIter->ModuleNames)
    {
    this->addClassifierDescriptor(dest, type, moduleName,
                                  infoIter->DescriptorName, value);
    }
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryDialogPrivate::addClassifierDescriptor(
  std::vector<vvDescriptor>& dest, int type, const QString& moduleName,
  const QString& descriptorName, double value) const
{
  vvDescriptor desc;

  desc.DescriptorName = stdString(descriptorName);
  desc.Values.clear();

  int minSize = qMax(vqEventInfo::staticTypeArraySize(), type + 1);
  desc.Values.push_back(std::vector<float>());
  desc.Values[0].assign(minSize, -1.0);

  desc.Values[0][type] = value;

  desc.ModuleName = stdString(moduleName);
  dest.push_back(desc);
}

//-----------------------------------------------------------------------------
vqPredefinedQueryDialog::vqPredefinedQueryDialog(QWidget* parent,
                                                 Qt::WindowFlags flags)
  : vvAbstractSimilarityQueryDialog(parent, flags),
    d_ptr(new vqPredefinedQueryDialogPrivate(this))
{
  QTE_D(vqPredefinedQueryDialog);

  d->Buttons = new QButtonGroup(this);
  connect(d->Buttons, SIGNAL(buttonPressed(int)),
          SLOT(selectQuery(int)));

  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Disallow accept with nothing selected
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  // Load queries (must do now so setSelectedDescriptors can work)
  d->loadQueries();

  if (d->setupClassifierGroup())
    {
    connect(d->UI.classifierGroup, SIGNAL(toggled(bool)),
            SLOT(toggleClassifierGroup(bool)));
    connect(d->UI.classifierFilter, SIGNAL(stateChanged(int, bool)),
            SLOT(enableAcceptIfClassifiersSelected()));

    }
}

//-----------------------------------------------------------------------------
vqPredefinedQueryDialog::~vqPredefinedQueryDialog()
{
}

//-----------------------------------------------------------------------------
int vqPredefinedQueryDialog::similarity() const
{
  QTE_D_CONST(vqPredefinedQueryDialog);
  if (d->UI.classifierGroup->isChecked())
    {
    return 10.0;  // 0.1 * 100
    }
  else if (d->CurrentQuery.isSimilarityQuery())
    {
    const double s =
      d->CurrentQuery.constSimilarityQuery()->SimilarityThreshold;
    return qRound(s * 100.0);
    }
  return -1;
}

//-----------------------------------------------------------------------------
std::vector<vvTrack> vqPredefinedQueryDialog::selectedTracks() const
{
  QTE_D_CONST(vqPredefinedQueryDialog);
  return (!d->UI.classifierGroup->isChecked() &&
          d->CurrentQuery.isSimilarityQuery()
          ? d->CurrentQuery.constSimilarityQuery()->Tracks
          : std::vector<vvTrack>());
}

//-----------------------------------------------------------------------------
std::vector<vvDescriptor> vqPredefinedQueryDialog::selectedDescriptors() const
{
  QTE_D_CONST(vqPredefinedQueryDialog);
  if (d->UI.classifierGroup->isChecked())
    {
    // build up vector of classifier descriptors and return them
    std::vector<vvDescriptor> classifierDescriptors;

    foreach (int type, d->UI.classifierFilter->keys())
      {
      if (d->UI.classifierFilter->state(type))
        {
        d->addClassifierDescriptors(classifierDescriptors, type,
                                    d->UI.classifierFilter->value(type));
        }
      }
    return classifierDescriptors;
    }
  return (d->CurrentQuery.isSimilarityQuery()
          ? d->CurrentQuery.constSimilarityQuery()->Descriptors
          : std::vector<vvDescriptor>());
}

//-----------------------------------------------------------------------------
std::vector<unsigned char> vqPredefinedQueryDialog::iqrModel() const
{
  QTE_D_CONST(vqPredefinedQueryDialog);
  return (!d->UI.classifierGroup->isChecked() &&
          d->CurrentQuery.isSimilarityQuery()
          ? d->CurrentQuery.constSimilarityQuery()->IqrModel
          : std::vector<unsigned char>());
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryDialog::setSelectedDescriptors(
  const std::vector<vvDescriptor>& descriptors)
{
  QTE_D(vqPredefinedQueryDialog);

  typedef QHash<int, vvQueryInstance>::const_iterator QueryIterator;
  foreach_iter (QueryIterator, iter, d->Queries)
    {
    const vvSimilarityQuery* sq = iter.value().constSimilarityQuery();
    if (sq && sq->Descriptors == descriptors)
      {
      d->Buttons->button(iter.key())->setChecked(true);
      this->selectQuery(iter.key());
      return;
      }
    }


  // Try to get exact match against Classifiers (if present)
  if (d->UI.classifierGroup->isHidden())
    {
    return;
    }

  bool fullMatch = true;
  std::vector<vvDescriptor> testDescriptors = descriptors;
  foreach (int type, d->UI.classifierFilter->keys())
    {
    std::vector<vvDescriptor> classifierDescriptors;
    d->addClassifierDescriptors(classifierDescriptors, type,
                                d->UI.classifierFilter->value(type));

    // see if EVERY descriptor in classifierDescriptors is in
    // testDescriptors (remove when match found)
    bool classifierMatched = true, partialMatch = false;
    std::vector<vvDescriptor>::iterator classifierIter, testIter;
    for (classifierIter = classifierDescriptors.begin();
         classifierIter != classifierDescriptors.end(); classifierIter++)
      {
      bool matchFound = false;
      for (testIter = testDescriptors.begin();
           testIter != testDescriptors.end(); testIter++)
        {
        if (*testIter == *classifierIter)
          {
          testDescriptors.erase(testIter);
          matchFound = true;
          partialMatch = true;
          break;
          }
        }
      if (!matchFound)
        {
        classifierMatched = false;
        break;
        }
      }

    // if completely matched, check the item; if partially matched, can't
    // exactly match incoming, thus clear all'
    if (classifierMatched)
      {
      d->UI.classifierFilter->setState(type, true);
      }
    else if (partialMatch)
      {
      fullMatch = false;
      break;
      }

    if (testDescriptors.empty())
      {
      break;
      }
    }

  // wasn't able to get exact mtching set - clear all
  if (!fullMatch || !testDescriptors.empty())
    {
    foreach (int type, d->UI.classifierFilter->keys())
      {
      d->UI.classifierFilter->setState(type, false);
      }
    }
  else // make sure Classifiers is checked (found exact match)
    {
    d->UI.classifierGroup->setChecked(true);
    d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
int vqPredefinedQueryDialog::exec()
{
  QTE_D(vqPredefinedQueryDialog);

  if (d->Queries.isEmpty() && d->PredefinedClassifiers.isEmpty())
    {
    QMessageBox::warning(this, QString(), d->LastError);
    return QDialog::Rejected;
    }

  // Qt does not allow a visible widget to become modal, so we must explicitly
  // set modality before calling show()
  const bool wasModal = this->isModal();
  this->setModal(true);
  this->show();

  // Calculate minimum or fixed size, depending on contents
  const QSize contentSize = d->UI.contentWidget->layout()->minimumSize();
  const QSize screenSize = QDesktopWidget().availableGeometry(this).size();
  d->UI.scrollArea->setMinimumWidth(contentSize.width());
  d->UI.scrollArea->setMaximumHeight(contentSize.height());
  if (contentSize.height() < 0.6 * screenSize.height())
    {
    d->UI.scrollArea->setMinimumSize(contentSize);
    this->setFixedSize(this->layout()->minimumSize());
    }
  else
    {
    const int wPad = this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int width = this->layout()->minimumSize().width() + wPad;
    const int height = qRound(0.6 * screenSize.height());
    d->UI.scrollArea->setMinimumHeight(height >> 1);
    this->setMaximumSize(this->layout()->maximumSize());
    this->setFixedWidth(width);
    this->resize(width, height);
    }

  const int result = QDialog::exec();
  this->setModal(wasModal);
  return result;
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryDialog::selectQuery(int id)
{
  QTE_D(vqPredefinedQueryDialog);
  d->CurrentQuery = d->Queries[id];
  QPushButton* acceptButton = d->UI.buttonBox->button(QDialogButtonBox::Ok);
  acceptButton->setEnabled(d->CurrentQuery.isValid());
  if (d->UI.classifierGroup->isChecked())
    {
    // if a radio button is selected, make sure Classifier GroupBox is not
    d->UI.classifierGroup->blockSignals(true);
    d->UI.classifierGroup->setChecked(false);
    d->UI.classifierGroup->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryDialog::toggleClassifierGroup(bool state)
{
  QTE_D(vqPredefinedQueryDialog);
  if (state && d->Buttons->checkedButton())
    {
    // radio button was checked; need to clear it as now Classifiers is checked
    d->Buttons->setExclusive(false);
    d->Buttons->checkedButton()->setChecked(false);
    d->Buttons->setExclusive(true);
    this->enableAcceptIfClassifiersSelected();
    }
  else
    {
    // don't allow unchecking of Classifiers by clicking on checkbox (must
    // select a radio button to do so)
    d->UI.classifierGroup->blockSignals(true);
    d->UI.classifierGroup->setChecked(true);
    d->UI.classifierGroup->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryDialog::enableAcceptIfClassifiersSelected()
{
  QTE_D(vqPredefinedQueryDialog);
  QPushButton* acceptButton = d->UI.buttonBox->button(QDialogButtonBox::Ok);
  bool classifierChecked = false;
  foreach (int type, d->UI.classifierFilter->keys())
    {
    if (d->UI.classifierFilter->state(type))
      {
      classifierChecked = true;
      break;
      }
    }
  acceptButton->setEnabled(classifierChecked);
}
