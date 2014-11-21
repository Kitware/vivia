/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsAlertEditor.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>

#include <qtScopedValueChange.h>
#include <qtUtil.h>

#include <vvQuery.h>
#include <vvReader.h>
#include <vvWriter.h>

#include <vgColor.h>
#include <vgFileDialog.h>

#include "vsAlert.h"

#include "ui_alertEditor.h"

#define die(_msg) return this->abort(fileName, _msg);
#define test_or_die(_cond, _msg) if (!(_cond)) die(_msg)

QTE_IMPLEMENT_D_FUNC(vsAlertEditor)

namespace // anonymous
{

//-----------------------------------------------------------------------------
QColor qColor(const double dc[3])
{
  return vgColor(dc[0], dc[1], dc[2]).toQColor();
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsAlertEditorPrivate
{
public:
  Ui::vsAlertEditor UI;
  bool uiInEditMode;

  vsEventInfo originalEventInfo;
  double originalDisplayThreshold;
  vvSimilarityQuery query;

  bool changed;
};

//-----------------------------------------------------------------------------
vsAlertEditor::vsAlertEditor(QWidget* parent)
  : QDialog(parent), d_ptr(new vsAlertEditorPrivate)
{
  QTE_D(vsAlertEditor);
  d->UI.setupUi(this);
  d->uiInEditMode = false;
  qtUtil::setStandardIcons(d->UI.buttonBox);

  connect(d->UI.name, SIGNAL(textChanged(QString)),
          this, SLOT(nameChanged()));
  connect(d->UI.queryFile, SIGNAL(textChanged(QString)),
          this, SLOT(queryFileChanged(QString)));
  connect(d->UI.penColor, SIGNAL(colorChanged(QColor)),
          this, SLOT(setModified()));
  connect(d->UI.foregroundColor, SIGNAL(colorChanged(QColor)),
          this, SLOT(setModified()));
  connect(d->UI.backgroundColor, SIGNAL(colorChanged(QColor)),
          this, SLOT(setModified()));
  connect(d->UI.displayThreshold, SIGNAL(valueChanged(double)),
          this, SLOT(setModified()));

  connect(d->UI.queryBrowse, SIGNAL(clicked()),
          this, SLOT(browseForQuery()));
  connect(d->UI.saveButton, SIGNAL(clicked()),
          this, SLOT(saveAlert()));

  d->UI.penColor->setColor(Qt::red);
  d->UI.foregroundColor->setColor(Qt::white);
  d->UI.backgroundColor->setColor(Qt::darkRed);
  this->apply();

  d->changed = false;
  this->validate();
}

//-----------------------------------------------------------------------------
vsAlertEditor::~vsAlertEditor()
{
}

//-----------------------------------------------------------------------------
void vsAlertEditor::setAlert(const vsAlert& alert)
{
  QTE_D(vsAlertEditor);
  d->originalEventInfo = alert.eventInfo;
  d->originalDisplayThreshold = alert.displayThreshold;
  d->query = alert.query;

  // Modify UI for editing (as opposed to creating) an alert, if we have not
  // already done so
  if (!d->uiInEditMode)
    {
    d->uiInEditMode = true;

    // Add 'apply', 'reset' buttons
    connect(d->UI.buttonBox->addButton(QDialogButtonBox::Apply),
            SIGNAL(clicked()), this, SLOT(apply()));
    connect(d->UI.buttonBox->addButton(QDialogButtonBox::Reset),
            SIGNAL(clicked()), this, SLOT(reset()));
    qtUtil::setStandardIcons(d->UI.buttonBox);

    // Hide query file controls
    d->UI.queryLabel->setVisible(false);
    d->UI.queryFile->setVisible(false);
    d->UI.queryBrowse->setVisible(false);
    }

  // Fill UI fields and set button states
  d->UI.displayThreshold->setValue(alert.displayThreshold);
  this->setQueryInfo();
  this->reset();
  this->validate();
  d->changed = false;
}

//-----------------------------------------------------------------------------
vsAlert vsAlertEditor::alert() const
{
  QTE_D_CONST(vsAlertEditor);
  vsAlert alert;
  alert.eventInfo = d->originalEventInfo;
  alert.query = d->query;
  alert.displayThreshold = d->originalDisplayThreshold;
  return alert;
}

//-----------------------------------------------------------------------------
void vsAlertEditor::selectName()
{
  QTE_D(vsAlertEditor);
  d->UI.name->setFocus();
  d->UI.name->selectAll();
}

//-----------------------------------------------------------------------------
void vsAlertEditor::accept()
{
  this->apply();
  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vsAlertEditor::reject()
{
  QTE_D_CONST(vsAlertEditor);
  if (d->changed)
    QDialog::accept();
  else
    QDialog::reject();
}

//-----------------------------------------------------------------------------
void vsAlertEditor::apply()
{
  QTE_D(vsAlertEditor);
  d->originalEventInfo.name = d->UI.name->text();
  vgColor::fillArray(d->UI.penColor->color(),
                     d->originalEventInfo.pcolor);
  vgColor::fillArray(d->UI.foregroundColor->color(),
                     d->originalEventInfo.fcolor);
  vgColor::fillArray(d->UI.backgroundColor->color(),
                     d->originalEventInfo.bcolor);
  d->originalDisplayThreshold = d->UI.displayThreshold->value();

  d->changed = true;
  this->setModified();
  emit this->changesApplied();
}

//-----------------------------------------------------------------------------
void vsAlertEditor::reset()
{
  QTE_D(vsAlertEditor);

  qtScopedBlockSignals bsN(d->UI.name);
  qtScopedBlockSignals bsPC(d->UI.penColor);
  qtScopedBlockSignals bsFC(d->UI.foregroundColor);
  qtScopedBlockSignals bsBC(d->UI.backgroundColor);
  qtScopedBlockSignals bsDT(d->UI.displayThreshold);

  d->UI.penColor->setColor(qColor(d->originalEventInfo.pcolor));
  d->UI.foregroundColor->setColor(qColor(d->originalEventInfo.fcolor));
  d->UI.backgroundColor->setColor(qColor(d->originalEventInfo.bcolor));
  d->UI.name->setText(d->originalEventInfo.name);
  d->UI.displayThreshold->setValue(d->originalDisplayThreshold);

  this->setModified();
}

//-----------------------------------------------------------------------------
void vsAlertEditor::setModified()
{
  QTE_D(vsAlertEditor);
  if (!(d->UI.buttonBox->button(QDialogButtonBox::Apply) &&
        d->UI.buttonBox->button(QDialogButtonBox::Reset)))
    return;

  bool modified = d->originalEventInfo.name != d->UI.name->text();
  modified |= (qColor(d->originalEventInfo.pcolor)
               != d->UI.penColor->color());
  modified |= (qColor(d->originalEventInfo.fcolor)
               != d->UI.foregroundColor->color());
  modified |= (qColor(d->originalEventInfo.bcolor)
               != d->UI.backgroundColor->color());
  modified |= (!qFuzzyCompare(d->originalDisplayThreshold,
                              d->UI.displayThreshold->value()));

  d->UI.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(modified);
  d->UI.buttonBox->button(QDialogButtonBox::Reset)->setEnabled(modified);
}

//-----------------------------------------------------------------------------
void vsAlertEditor::validate()
{
  QTE_D(vsAlertEditor);

  bool valid = !d->UI.name->text().isEmpty();
  valid = valid && (!d->UI.queryFile->isVisible()
                    || d->query.Descriptors.size());

  d->UI.saveButton->setEnabled(valid);
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

//-----------------------------------------------------------------------------
void vsAlertEditor::setQueryInfo(QString error)
{
  QTE_D(vsAlertEditor);

  if (error.isEmpty())
    {
    d->UI.queryInfo->setErrorText("(no descriptors)");
    d->UI.queryInfo->setQuery(d->query);
    }
  else
    {
    d->UI.queryInfo->setErrorText(error);
    d->UI.queryInfo->clearData();
    }
}

//-----------------------------------------------------------------------------
bool vsAlertEditor::loadAlert(const QString& fileName)
{
  if (!fileName.isEmpty() && QFileInfo(fileName).exists())
    {
    vvReader reader;
    test_or_die(reader.open(QUrl::fromLocalFile(fileName)),
                reader.error());

    vvHeader header;
    test_or_die(reader.readHeader(header), reader.error());

    test_or_die(header.type == vvHeader::EventSetInfo,
                "The file does not appear to be a saved alert");

    vvEventSetInfo info;
    test_or_die(reader.readEventSetInfo(info), reader.error());

    if (this->readQuery(reader, fileName))
      {
      QTE_D(vsAlertEditor);

      d->UI.name->setText(info.Name);
      d->UI.penColor->setColor(info.PenColor);
      d->UI.foregroundColor->setColor(info.ForegroundColor);
      d->UI.backgroundColor->setColor(info.BackgroundColor);
      d->UI.displayThreshold->setValue(info.DisplayThreshold);
      this->apply();
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
void vsAlertEditor::saveAlert()
{
  QString path = vgFileDialog::getSaveFileName(
                   this, "Save alert...", QString(),
                   "VisGUI Saved Alert (*.vsa *.vsax);;"
                   "VisGUI Saved Alert - KST (*.vsa);;"
                   "VisGUI Saved Alert - XML (*.vsax *.xml);;"
                   "All files (*)");

  if (!path.isEmpty())
    {
    QTE_D(vsAlertEditor);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
      qDebug() << "Error opening file" << path << "for writing:"
               << qPrintable(file.errorString());
      return;
      }

    vvEventSetInfo info;
    info.Name = d->UI.name->text();
    info.PenColor = d->UI.penColor->color();
    info.ForegroundColor = d->UI.foregroundColor->color();
    info.BackgroundColor = d->UI.backgroundColor->color();
    info.DisplayThreshold = d->UI.displayThreshold->value();

    const vvWriter::Format format =
      QFileInfo(path).suffix().contains('x') ? vvWriter::Xml : vvWriter::Kst;
    QTextStream stream(&file);
    vvWriter writer(stream, format);
    writer << vvHeader::EventSetInfo << info
           << vvHeader::QueryPlan << vvQueryInstance(d->query);
    }
}

//-----------------------------------------------------------------------------
bool vsAlertEditor::readQuery(vvReader& reader, const QString& fileName)
{
  QTE_D(vsAlertEditor);

  vvHeader header;

  test_or_die(reader.readHeader(header), reader.error());

  test_or_die(header.type == vvHeader::QueryPlan,
              "The file does not contain a query plan");

  vvQueryInstance query;
  test_or_die(reader.readQueryPlan(query), reader.error());
  test_or_die(query.isSimilarityQuery(),
              "Non-similarity queries cannot be used");

  d->query = *query.similarityQuery();
  this->setQueryInfo();
  return true;
}

//-----------------------------------------------------------------------------
bool vsAlertEditor::abort(const QString& fileName, const QString& message)
{
  return this->abort(fileName, qPrintable(message));
}

//-----------------------------------------------------------------------------
bool vsAlertEditor::abort(const QString& fileName, const char* message)
{
  const QString errorPrefix("Error reading file \"%1\":");
  qDebug() << qPrintable(errorPrefix.arg(fileName)) << message;
  this->setQueryInfo("(query file is not valid)");
  return false;
}

//-----------------------------------------------------------------------------
void vsAlertEditor::browseForQuery()
{
  QString path = vgFileDialog::getOpenFileName(
                   this, "Browse for query...", QString(),
                   "VisGUI Query Plans (*.vqp);;"
                   "All files (*)");

  if (!path.isEmpty())
    {
    QTE_D(vsAlertEditor);
    d->UI.queryFile->setText(path);
    }
}

//-----------------------------------------------------------------------------
void vsAlertEditor::nameChanged()
{
  this->setModified();
  this->validate();
}

//-----------------------------------------------------------------------------
void vsAlertEditor::queryFileChanged(QString fileName)
{
  QTE_D(vsAlertEditor);
  d->query.Descriptors.clear();

  QFileInfo fi(fileName);
  if (fi.exists())
    {
    vvReader reader;
    if (!reader.open(QUrl::fromLocalFile(fileName)))
      {
      this->abort(fileName, reader.error());
      return;
      }

    if (!this->readQuery(reader, fileName))
      {
      return;
      }
    }
  else if (fileName.isEmpty())
    {
    this->setQueryInfo("(no query provided)");
    }
  else
    {
    this->setQueryInfo("(query file does not exist)");
    }

  this->validate();
}
