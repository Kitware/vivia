/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqQueryDialog.h"
#include "ui_query.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

#include <qtKstReader.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <vgGeodesy.h>

#include <vgFileDialog.h>
#include <vgGeoUtil.h>
#include <vgUnixTime.h>

#include <vvMakeId.h>
#include <vvKstReader.h>
#include <vvKstWriter.h>

#include <vvAbstractSimilarityQueryDialog.h>

#include <cmath>

#include "vqClassifierQueryDialog.h"
#include "vqCore.h"
#include "vqPredefinedQueryDialog.h"
#include "vqQueryParser.h"
#include "vqRegionEditDialog.h"
#include "vqUtil.h"
#include "vqVideoQueryDialog.h"

QTE_IMPLEMENT_D_FUNC(vqQueryDialog)

//BEGIN vqQueryDialogPrivate

//-----------------------------------------------------------------------------
class vqQueryDialogPrivate
{
public:
  enum QueryType
    {
    Unspecified         = 0,
    ImageQuery          = 0x10,
    VideoQuery          = 0x11,
    DatabaseQuery       = 0x1e,
    ExemplarQuery       = 0x1f,
    ClassifierQuery     = 0x20,
    TrackQuery          = 0x40,
    SavedQuery          = 0x80,
    PredefinedQuery     = 0x81
    };

  struct Exemplar
    {
    std::string Uri;
    std::vector<vvDescriptor> Descriptors;
    };

  vqQueryDialogPrivate(vqQueryDialog* q) : q_ptr(q) {}

  void savePersistentQuery();

  void setLastQuery(const std::string& uri,
                    const std::vector<vvDescriptor>& descriptors);
  void clearLastQuery();

  bool editSimilarityQuery(vvAbstractSimilarityQueryDialog* dialog,
                           int forceSimilarity,
                           std::vector<vvDescriptor>& selectedDescriptors);

  void editExemplarQuery(Exemplar&, vvQueryFormulationType);
  void editPredefinedQuery();
  void editClassifierQuery();
  void editTrackQuery();

  void setQueryRegion(vgGeocodedPoly region);

  void updateQuery();
  void fixupQuery();

  static QString dmsString(double coord, char dp, char dn);

  Ui::vqQueryDialog UI;
  vqCore* core_;
  vvQueryInstance query_;
  QString error_;

  Exemplar LastVideoQuery;
  Exemplar LastImageQuery;
  Exemplar LastDatabaseQuery;
  Exemplar LastExemplarQuery;
  std::vector<vvDescriptor> LastPredefinedQueryDescriptors;
  std::vector<vvDescriptor> LastClassifierQueryDescriptors;

  QueryType queryType_;
  bool editTypeOnIndexChange_;

  bool UseAdvancedUi;

protected:
  QTE_DECLARE_PUBLIC_PTR(vqQueryDialog)

private:
  QTE_DECLARE_PUBLIC(vqQueryDialog)
};

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::setLastQuery(
  const std::string& uri, const std::vector<vvDescriptor>& descriptors)
{
  this->LastImageQuery.Uri = uri;
  this->LastImageQuery.Descriptors = descriptors;
  this->LastVideoQuery.Uri = uri;
  this->LastVideoQuery.Descriptors = descriptors;
  this->LastExemplarQuery.Uri = uri;
  this->LastExemplarQuery.Descriptors = descriptors;
  this->LastPredefinedQueryDescriptors = descriptors;
  this->LastClassifierQueryDescriptors = descriptors;
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::clearLastQuery()
{
  this->LastImageQuery.Uri.clear();
  this->LastImageQuery.Descriptors.clear();
  this->LastVideoQuery.Uri.clear();
  this->LastVideoQuery.Descriptors.clear();
  this->LastExemplarQuery.Uri.clear();
  this->LastExemplarQuery.Descriptors.clear();
  this->LastPredefinedQueryDescriptors.clear();
  this->LastClassifierQueryDescriptors.clear();
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::savePersistentQuery()
{
  QSettings settings;
  settings.beginGroup("Query");

  // Save query type
  settings.setValue("Type", this->queryType_);

  // Save query
  QString data;
  QTextStream stream(&data, QIODevice::WriteOnly);
  vvKstWriter writer(stream, false);
  stream << vvKstWriter::QueryPlanVersion << ';';
  writer << this->query_;
  settings.setValue("Data", data);

  // Save relevancy (in case the query is a retrieval query)
  settings.setValue("Relevancy", this->UI.relevancySpin->value());

  // Save spatial limit enabled/disabled state (since we store the filter mode
  // in the query data, above, regardless of enabled/disabled)
  settings.setValue("UseSpatialLimit", this->UI.regionLimit->isChecked());

  // Save if the IQR model (if present) should be used
  settings.setValue("UseIqrModel", this->UI.useIqrModel->isChecked());
}

//-----------------------------------------------------------------------------
bool vqQueryDialogPrivate::editSimilarityQuery(
  vvAbstractSimilarityQueryDialog* dialog, int forceSimilarity,
  std::vector<vvDescriptor>& selectedDescriptors)
{
  QTE_Q(vqQueryDialog);

  dialog->setSelectedDescriptors(selectedDescriptors);
  if (dialog->exec())
    {
    this->query_ = vvSimilarityQuery(*this->query_.abstractQuery());
    vvSimilarityQuery& query = *this->query_.similarityQuery();
    query.StreamIdLimit.clear();
    query.Tracks = dialog->selectedTracks();
    query.Descriptors = dialog->selectedDescriptors();
    selectedDescriptors = query.Descriptors;
    q->resetQueryId();

    // \TODO: remove when user set relevancy is functional
    this->UI.relevancySpin->setValue(forceSimilarity);
    this->UI.relevancySlider->setEnabled(true);
    this->UI.relevancySpin->setEnabled(true);

    this->updateQuery();
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::editExemplarQuery(
  Exemplar& exemplar, vvQueryFormulationType type)
{
  QTE_Q(vqQueryDialog);

  vqVideoQueryDialog vqDialog(this->UseAdvancedUi, q);
  vqDialog.initialize();

  QObject::connect(this->core_,
                   SIGNAL(formulationComplete(QList<vvDescriptor>,
                                              QList<vvTrack>)),
                   &vqDialog,
                   SLOT(setQueryTracksAndDescriptors(QList<vvDescriptor>,
                        QList<vvTrack>)));

  QObject::connect(this->core_,
                   SIGNAL(formulationFailed()),
                   &vqDialog,
                   SLOT(clearQueryDescriptors()));

  QObject::connect(&vqDialog,
                   SIGNAL(queryFormulationRequested(vvProcessingRequest,
                                                    bool,
                                                    qtStatusManager*)),
                   this->core_,
                   SLOT(formulateQuery(vvProcessingRequest,
                                       bool, qtStatusManager*)));

  // \TODO ideally this if () should not be needed...
  if (type == vvQueryFormulation::FromDatabase)
    {
    const QUrl uri = qtUrl(this->LastDatabaseQuery.Uri);

    // \TODO need to pass limits as part of URI
    QUrl bareUri = uri;
    bareUri.setEncodedQuery(QByteArray());
    vqDialog.setExemplarUri(stdString(bareUri), type);

    // \TODO all of this should be done in the QF dialog
    vvQueryInstance qi(vvDatabaseQuery::Retrieval);
    vvRetrievalQuery* query = qi.retrievalQuery();

    query->QueryId = vvMakeId("VIQUI-QF");
    query->StreamIdLimit = stdString(bareUri);
    query->TemporalLowerLimit = uri.queryItemValue("StartTime").toLongLong();
    query->TemporalUpperLimit = uri.queryItemValue("EndTime").toLongLong();
    query->RequestedEntities = vvRetrievalQuery::TracksAndDescriptors;

    emit q->readyToProcessDatabaseVideoQuery(qi);

    vqDialog.setTimeRange(query->TemporalLowerLimit,
                          query->TemporalUpperLimit,
                          uri.queryItemValue("InitialTime").toDouble());
    }
  else
    {
    vqDialog.setExemplarUri(exemplar.Uri, type);
    }

  if (this->editSimilarityQuery(&vqDialog, 0, exemplar.Descriptors))
    {
    vvSimilarityQuery& query = *this->query_.similarityQuery();
    query.StreamIdLimit = vqDialog.exemplarUri();
    exemplar.Uri = query.StreamIdLimit;
    this->updateQuery();
    }
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::editPredefinedQuery()
{
  QTE_Q(vqQueryDialog);

  vqPredefinedQueryDialog pqDialog(q);

  std::vector<vvDescriptor>& descriptors =
    this->LastPredefinedQueryDescriptors;
  if (this->editSimilarityQuery(&pqDialog, 0, descriptors))
    {
    vvSimilarityQuery& query = *this->query_.similarityQuery();
    query.IqrModel = pqDialog.iqrModel();
    // \TODO: remove when user set relevancy is functional
    this->UI.relevancySpin->setValue(pqDialog.similarity());
    this->updateQuery();
    }
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::editClassifierQuery()
{
  QTE_Q(vqQueryDialog);

  vqClassifierQueryDialog cqDialog(q);
  this->editSimilarityQuery(&cqDialog, 10,
                            this->LastClassifierQueryDescriptors);
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::editTrackQuery()
{
  QTE_Q(vqQueryDialog);

  this->query_ = vvRetrievalQuery(*this->query_.abstractQuery());
  vvRetrievalQuery& query = *this->query_.retrievalQuery();
  query.StreamIdLimit.clear();
  query.RequestedEntities = vvRetrievalQuery::Tracks;
  q->resetQueryId();

  this->UI.relevancySlider->setEnabled(false);
  this->UI.relevancySpin->setEnabled(false);
  this->updateQuery();
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::setQueryRegion(vgGeocodedPoly region)
{
  QTE_Q(vqQueryDialog);

  const vgGeocodedCoordinate coord = vgGeodesy::regionCenter(region);
  this->UI.region->setText(vgGeodesy::coordString(coord));

  this->query_.abstractQuery()->SpatialLimit = region;
  q->resetQueryId();
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::updateQuery()
{
  const vvSimilarityQuery* sq = this->query_.constSimilarityQuery();
  const bool enable =
    this->query_.isRetrievalQuery() || (sq && sq->Descriptors.size());
  const bool haveIqrModel = sq && !sq->IqrModel.empty();

  this->UI.queryInfo->setQuery(this->query_);
  this->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
  this->UI.buttonSave->setEnabled(enable);
  this->UI.useIqrModel->setEnabled(haveIqrModel);
  this->UI.useIqrModel->setChecked(haveIqrModel);
}

//-----------------------------------------------------------------------------
void vqQueryDialogPrivate::fixupQuery()
{
  vvDatabaseQuery* query = this->query_.abstractQuery();
  vvSimilarityQuery* squery = this->query_.similarityQuery();

  // Set query ID if invalid
  if (query->QueryId.empty())
    {
    query->QueryId = vvMakeId("VIQUI");
    }

  // Set relevancy threshold
  if (squery)
    {
    squery->SimilarityThreshold = 0.01 * this->UI.relevancySpin->value();
    }

  // Set geospatial filter mode
  if (this->UI.enterEvent->isChecked())
    {
    query->SpatialFilter = vvDatabaseQuery::IntersectsInbound;
    }
  else if (this->UI.exitEvent->isChecked())
    {
    query->SpatialFilter = vvDatabaseQuery::IntersectsOutbound;
    }
  else if (this->UI.crossEvent->isChecked())
    {
    query->SpatialFilter = vvDatabaseQuery::Intersects;
    }
  else
    {
    query->SpatialFilter = vvDatabaseQuery::ContainsAny;
    }

  // Set temporal limit
  query->TemporalLowerLimit =
    vgUnixTime(this->UI.timeLower->dateTime()).toInt64();
  query->TemporalUpperLimit =
    vgUnixTime(this->UI.timeUpper->dateTime()).toInt64();

  if (this->UI.timeLimit->isChecked())
    {
    query->TemporalFilter = vvDatabaseQuery::ContainsAny;
    }
  else
    {
    query->TemporalFilter = vvDatabaseQuery::Ignore;
    }
}

//END vqQueryDialogPrivate

///////////////////////////////////////////////////////////////////////////////

//BEGIN vqQueryDialog

//-----------------------------------------------------------------------------
vqQueryDialog::vqQueryDialog(vqCore* core, bool useAdvancedUi,
                             QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags), d_ptr(new vqQueryDialogPrivate(this))
{
  QTE_D(vqQueryDialog);

  d->core_ = core;
  d->queryType_ = vqQueryDialogPrivate::Unspecified;
  d->editTypeOnIndexChange_ = false;
  d->UseAdvancedUi = useAdvancedUi;

  d->UI.setupUi(this);
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Set default min/max values for temporal limit, and default initial value
  QDateTime epoch = QDateTime::fromMSecsSinceEpoch(0).toUTC();
  d->UI.timeLower->setMinimumDateTime(epoch.addYears(-40));
  d->UI.timeUpper->setMinimumDateTime(epoch.addYears(-40));
  d->UI.timeLower->setMaximumDateTime(epoch.addYears(530).addSecs(-1));
  d->UI.timeUpper->setMaximumDateTime(epoch.addYears(530).addSecs(-1));
  QDateTime now = QDateTime::currentDateTime();
  d->UI.timeUpper->setDateTime(now);
  d->UI.timeLower->setDateTime(now.addMonths(-1));

  // Add primary query types
  d->UI.queryType->addItem("Video Exemplar",
                           vqQueryDialogPrivate::VideoQuery);
  d->UI.queryType->addItem("Image Exemplar",
                           vqQueryDialogPrivate::ImageQuery);
  d->UI.queryType->addItem("System Predefined",
                           vqQueryDialogPrivate::PredefinedQuery);
  d->UI.queryType->addItem("User Saved",
                           vqQueryDialogPrivate::SavedQuery);

  // Connect primary UI actions
  connect(d->UI.queryType, SIGNAL(highlighted(int)),
          this, SLOT(preSetQueryType()));
  connect(d->UI.queryType, SIGNAL(activated(int)),
          this, SLOT(setQueryType(int)));
  connect(d->UI.pickRegionGraphic, SIGNAL(clicked()),
          this, SLOT(requestRegion()));
  connect(d->UI.pickRegionNumeric, SIGNAL(clicked()),
          this, SLOT(editRegionNumeric()));

  connect(d->UI.editQuery, SIGNAL(clicked()),
          this, SLOT(editQuery()));

  connect(d->UI.buttonLoad, SIGNAL(clicked()),
          this, SLOT(loadQuery()));
  connect(d->UI.buttonSave, SIGNAL(clicked()),
          this, SLOT(saveQuery()));

  // Connect correlated UI controls
  connect(d->UI.timeLower, SIGNAL(editingFinished()),
          this, SLOT(updateTimeUpperFromLower()));
  connect(d->UI.timeUpper, SIGNAL(editingFinished()),
          this, SLOT(updateTimeLowerFromUpper()));

  // Reset query ID when query changed, for UI controls that directly manipulate
  // the query
  connect(d->UI.relevancySpin, SIGNAL(valueChanged(int)),
          this, SLOT(resetQueryId()));

  connect(d->UI.timeLower, SIGNAL(dateTimeChanged(const QDateTime&)),
          this, SLOT(resetQueryId()));
  connect(d->UI.timeUpper, SIGNAL(dateTimeChanged(const QDateTime&)),
          this, SLOT(resetQueryId()));
  connect(d->UI.timeLimit, SIGNAL(toggled(bool)),
          this, SLOT(resetQueryId()));

  connect(d->UI.regionLimit, SIGNAL(toggled(bool)),
          this, SLOT(resetQueryId()));
  connect(d->UI.enterEvent, SIGNAL(toggled(bool)),
          this, SLOT(resetQueryId()));
  connect(d->UI.exitEvent, SIGNAL(toggled(bool)),
          this, SLOT(resetQueryId()));
  connect(d->UI.crossEvent, SIGNAL(toggled(bool)),
          this, SLOT(resetQueryId()));
  connect(d->UI.anyEvent, SIGNAL(toggled(bool)),
          this, SLOT(resetQueryId()));

  // GUI spatial filtering options disabled for now
  d->UI.enterEvent->setVisible(false);
  d->UI.exitEvent->setVisible(false);
  d->UI.crossEvent->setVisible(false);
  d->UI.anyEvent->setVisible(false);

  // Set up initial query
  d->query_ = vvDatabaseQuery();
  d->updateQuery();

  // Set up UI for Analyst/Engineering mode
  if (useAdvancedUi)
    {
    // ENGINEERING mode
    // Add advanced query types
    d->UI.queryType->addItem("Track Retrieval",
                             vqQueryDialogPrivate::TrackQuery);
    d->UI.queryType->addItem("Exemplar (Advanced)",
                             vqQueryDialogPrivate::ExemplarQuery);
    d->UI.queryType->addItem("Classifier (Advanced)",
                             vqQueryDialogPrivate::ClassifierQuery);
    }
  else
    {
    // ANALYST mode
    // Hide load/save controls in Analyst mode
    d->UI.buttonLoad->hide();
    d->UI.buttonSave->hide();
    }

  // \TODO: remove when user set relevancy is functional
  // Right now, the relevancy score is not meaningful, except that it must be
  // within a certain value that is NOT [0,1]. So, hide the UI for now. We will
  // still use the UI controls for internal synchronization.
  d->UI.relevancySlider->hide();
  d->UI.relevancySpin->hide();
  d->UI.labelRelevancy->hide();
  this->resize(this->layout()->minimumSize());
}

//-----------------------------------------------------------------------------
vqQueryDialog::~vqQueryDialog()
{
}

//-----------------------------------------------------------------------------
void vqQueryDialog::accept()
{
  QTE_D(vqQueryDialog);

  d->fixupQuery();
  d->savePersistentQuery();

  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vqQueryDialog::initiateDatabaseQuery(
  std::string videoUri, long long startTimeLimit, long long endTimeLimit,
  long long initialTime)
{
  QTE_D(vqQueryDialog);

  QUrl uri = qtUrl(videoUri);
  uri.addQueryItem("StartTime",   QString::number(startTimeLimit));
  uri.addQueryItem("EndTime",     QString::number(endTimeLimit));
  uri.addQueryItem("InitialTime", QString::number(initialTime));

  d->LastDatabaseQuery.Descriptors.clear();
  d->LastDatabaseQuery.Uri = stdString(uri);

  d->queryType_ = vqQueryDialogPrivate::DatabaseQuery;

  // \NOTE Disabled for now... \TODO revisit this decision in future
  // \TODO Add this functionality in general from query type drop down?
  d->UI.editQuery->setEnabled(false);
  d->UI.queryType->setEnabled(false);

  this->editQuery();
}

//-----------------------------------------------------------------------------
void vqQueryDialog::loadPersistentQuery()
{
  QTE_D(vqQueryDialog);

  vvKstReader reader;
  QString data;

  QSettings settings;
  settings.beginGroup("Query");

  // Reload query type
  QVariant queryType =
    d->UI.queryType->itemData(d->UI.queryType->currentIndex());
  queryType = settings.value("Type", queryType);
  d->UI.queryType->setCurrentIndex(d->UI.queryType->findData(queryType));
  this->setQueryType(d->UI.queryType->currentIndex());

  // Clear old descriptors and exemplar URI's
  d->clearLastQuery();

  // Reload query
  vvQueryInstance query;
  data = settings.value("Data").toString();
  if (!data.isEmpty())
    {
    qtKstReader kst(data);
    int version;
    if (kst.readInt(version) && kst.nextRecord()
        && reader.readQueryPlan(kst, query, version))
      {
      this->setQuery(query, false);
      }
    }

  // Reload relevancy
  d->UI.relevancySpin->setValue(
    settings.value("Relevancy", d->UI.relevancySpin->value()).toInt());

  // Reload spatial limit toggle
  d->UI.regionLimit->setChecked(
    settings.value("UseSpatialLimit", false).toBool());

  // Reload IQR model toggle
  d->UI.useIqrModel->setChecked(
    settings.value("UseIqrModel", false).toBool());

  // Reset query ID (will have been cleared by setting UI controls to new query)
  if (query.isValid())
    {
    d->query_.abstractQuery()->QueryId = query.abstractQuery()->QueryId;
    }
}

//-----------------------------------------------------------------------------
void vqQueryDialog::savePersistentQuery(const vvQueryInstance& query)
{
  QSettings settings;
  settings.beginGroup("Query");

  // Save query type
  settings.setValue("Type", vqQueryDialogPrivate::SavedQuery);

  // Save query
  QString data;
  QTextStream stream(&data, QIODevice::WriteOnly);
  vvKstWriter writer(stream, false);
  stream << vvKstWriter::QueryPlanVersion << ';';
  writer << query;
  settings.setValue("Data", data);

  // Don't store independent relevancy, as it isn't available
  settings.remove("Relevancy");

  // Spatial limit option is persisted separate from the filter mode in the
  // query, but for this purpose, if the query did not enable filtering, that
  // information is unfortunately lost... therefore, if the query had a spatial
  // filter mode, use it
  const vvDatabaseQuery* q = query.constAbstractQuery();
  settings.setValue("UseSpatialLimit",
                    q && q->SpatialFilter != vvDatabaseQuery::Ignore);
}

//-----------------------------------------------------------------------------
vvQueryInstance vqQueryDialog::query()
{
  QTE_D(vqQueryDialog);

  d->fixupQuery();
  vvQueryInstance query = d->query_;

  // Set geospatial limit (actually, clear if disabled; it is already set)
  if (!d->UI.regionLimit->isChecked())
    {
    query.abstractQuery()->SpatialLimit.GCS = -1;
    query.abstractQuery()->SpatialLimit.Coordinate.clear();
    query.abstractQuery()->SpatialFilter = vvDatabaseQuery::Ignore;
    }

  // Set temporal limit (actually, clear if disabled; it is already set)
  if (!d->UI.timeLimit->isChecked())
    {
    query.abstractQuery()->TemporalLowerLimit = -1;
    query.abstractQuery()->TemporalUpperLimit = -1;
    query.abstractQuery()->TemporalFilter = vvDatabaseQuery::Ignore;
    }

  // Clear IqrModel if there is one (useIqrModel enabled) but don't want to use
  if (d->UI.useIqrModel->isEnabled() && !d->UI.useIqrModel->isChecked())
    {
    query.similarityQuery()->IqrModel.clear();
    }

  // Done
  return query;
}

//-----------------------------------------------------------------------------
void vqQueryDialog::preSetQueryType()
{
  QTE_D(vqQueryDialog);
  d->editTypeOnIndexChange_ = true;
}

//-----------------------------------------------------------------------------
void vqQueryDialog::setQueryType(int index)
{
  QTE_D(vqQueryDialog);

  const int newType = d->UI.queryType->itemData(index).toInt();

  bool edit = d->editTypeOnIndexChange_;
  d->editTypeOnIndexChange_ = false;

  if (newType == vqQueryDialogPrivate::Unspecified)
    {
    d->UI.queryType->blockSignals(true);
    index = d->UI.queryType->findData(d->queryType_);
    d->UI.queryType->setCurrentIndex(index);
    d->UI.queryType->blockSignals(false);
    return;
    }

  if (newType != d->queryType_)
    {
    if (newType == vqQueryDialogPrivate::TrackQuery)
      {
      d->UI.editQuery->setEnabled(false);
      d->editTrackQuery();
      }
    else
      {
      d->query_ = vvSimilarityQuery(*d->query_.abstractQuery());
      vvSimilarityQuery& query = *d->query_.similarityQuery();
      switch (newType)
        {
        case vqQueryDialogPrivate::ImageQuery:
          query.StreamIdLimit = d->LastImageQuery.Uri;
          query.Descriptors = d->LastImageQuery.Descriptors;
          break;
        case vqQueryDialogPrivate::VideoQuery:
          query.StreamIdLimit = d->LastVideoQuery.Uri;
          query.Descriptors = d->LastVideoQuery.Descriptors;
          break;
        case vqQueryDialogPrivate::ExemplarQuery:
          query.StreamIdLimit = d->LastExemplarQuery.Uri;
          query.Descriptors = d->LastExemplarQuery.Descriptors;
          break;
        case vqQueryDialogPrivate::PredefinedQuery:
          query.StreamIdLimit.clear();
          query.Descriptors = d->LastPredefinedQueryDescriptors;
        case vqQueryDialogPrivate::ClassifierQuery:
          query.StreamIdLimit.clear();
          query.Descriptors = d->LastClassifierQueryDescriptors;
        default:
          query.StreamIdLimit.clear();
          break;
        }
      this->resetQueryId();
      d->UI.editQuery->setEnabled(true);
      }
    d->queryType_ = static_cast<vqQueryDialogPrivate::QueryType>(newType);
    d->updateQuery();
    }

  if (edit)
    this->editQuery();
}

//-----------------------------------------------------------------------------
void vqQueryDialog::editQuery()
{
  QTE_D(vqQueryDialog);

  switch (d->queryType_)
    {
    case vqQueryDialogPrivate::ImageQuery:
      d->editExemplarQuery(d->LastImageQuery, vvQueryFormulation::FromImage);
      break;
    case vqQueryDialogPrivate::VideoQuery:
      d->editExemplarQuery(d->LastVideoQuery, vvQueryFormulation::FromVideo);
      break;
    case vqQueryDialogPrivate::DatabaseQuery:
      d->editExemplarQuery(d->LastDatabaseQuery,
                           vvQueryFormulation::FromDatabase);
      break;
    case vqQueryDialogPrivate::ExemplarQuery:
      d->editExemplarQuery(d->LastExemplarQuery,
                           vvQueryFormulation::UserDefined);
      break;
    case vqQueryDialogPrivate::ClassifierQuery:
      d->editClassifierQuery();
      break;
    case vqQueryDialogPrivate::TrackQuery:
      d->editTrackQuery();
      break;
    case vqQueryDialogPrivate::PredefinedQuery:
      d->editPredefinedQuery();
      break;
    case vqQueryDialogPrivate::SavedQuery:
      this->loadQuery();
      break;
    default:
      break;
    }
}

//-----------------------------------------------------------------------------
void vqQueryDialog::loadQuery()
{
  QString fileName = vgFileDialog::getOpenFileName(
                       this, "Load saved query...", QString(),
                       "VisGUI Query Plans (*.vqp *.vqpx);;"
                       "All files (*)");

  if (!fileName.isEmpty())
    {
    QTE_D(vqQueryDialog);

    vqQueryParser parser;
    connect(&parser, SIGNAL(planAvailable(vvQueryInstance)),
            this, SLOT(setQuery(vvQueryInstance)));
    connect(&parser, SIGNAL(error(qtStatusSource, QString)),
            this, SLOT(setError(qtStatusSource, QString)));
    d->error_.clear();

    if (!parser.loadQuery(QUrl::fromLocalFile(fileName)))
      {
      QString msg = "A problem occurred while reading the query file.\n\n"
                    "Parser output:\n" + d->error_;
      QMessageBox::critical(this, "Error loading query", msg);
      }
    }
}

//-----------------------------------------------------------------------------
void vqQueryDialog::setQuery(vvQueryInstance query, bool setType)
{
  QTE_D(vqQueryDialog);

  d->query_ = query;

  const vvDatabaseQuery& aq = *d->query_.constAbstractQuery();
  const vvRetrievalQuery* rq = d->query_.constRetrievalQuery();
  const vvSimilarityQuery* sq = d->query_.constSimilarityQuery();

  // May load a query from QueryDialog that already has a loaded query (which
  // had an IQR model, thus disabling editQuery and queryType)
  d->UI.editQuery->setEnabled(true);
  d->UI.queryType->setEnabled(true);

  // Set UI query type (if loading from file, i.e. not persistent query)
  if (setType)
    {
    if (rq && rq->RequestedEntities == vvRetrievalQuery::Tracks)
      {
      d->queryType_ = vqQueryDialogPrivate::TrackQuery;
      const int index = d->UI.queryType->findData(d->queryType_);
      d->UI.queryType->setCurrentIndex(index);
      d->UI.editQuery->setEnabled(false);
      }
    else
      {
      d->queryType_ = vqQueryDialogPrivate::SavedQuery;
      const int index = d->UI.queryType->findData(d->queryType_);
      d->UI.queryType->setCurrentIndex(index);
      // Try to use this query's descriptors to preset query type descriptors
      d->setLastQuery(sq->StreamIdLimit, sq->Descriptors);
      }
    }
  else if (sq)
    {
    // If loading persistent query, since we know the query type, only set
    // query descriptors for the type of query this actually is
    switch (d->queryType_)
      {
      case vqQueryDialogPrivate::ImageQuery:
        d->LastImageQuery.Uri = sq->StreamIdLimit;
        d->LastImageQuery.Descriptors = sq->Descriptors;
        break;
      case vqQueryDialogPrivate::VideoQuery:
        d->LastVideoQuery.Uri = sq->StreamIdLimit;
        d->LastVideoQuery.Descriptors = sq->Descriptors;
        break;
      case vqQueryDialogPrivate::ExemplarQuery:
        d->LastExemplarQuery.Uri = sq->StreamIdLimit;
        d->LastExemplarQuery.Descriptors = sq->Descriptors;
        break;
      case vqQueryDialogPrivate::PredefinedQuery:
        d->LastPredefinedQueryDescriptors = sq->Descriptors;
      case vqQueryDialogPrivate::ClassifierQuery:
        d->LastClassifierQueryDescriptors = sq->Descriptors;
      default:
        break;
      }
    }
  d->updateQuery();

  // Set UI similarity
  if (sq)
    {
    d->UI.relevancySpin->setValue(100.0 * sq->SimilarityThreshold);
    }

  // Set UI temporal limit
  d->UI.timeLimit->setChecked(
    aq.TemporalFilter != vvDatabaseQuery::Ignore);
  QDateTime temporalLower = vgUnixTime(aq.TemporalLowerLimit).toDateTime();
  QDateTime temporalUpper = vgUnixTime(aq.TemporalUpperLimit).toDateTime();
  d->UI.timeLower->setDateTime(temporalLower.toUTC());
  d->UI.timeUpper->setDateTime(temporalUpper.toUTC());

  // Set UI geospatial limit and intersect option
  if (aq.SpatialLimit.Coordinate.size())
    {
    d->core_->setRegion(aq.SpatialLimit);
    d->setQueryRegion(aq.SpatialLimit);
    }
  d->UI.regionLimit->setChecked(true);
  switch (aq.SpatialFilter)
    {
    case vvDatabaseQuery::Ignore:
      d->UI.regionLimit->setChecked(false);
      break;
    case vvDatabaseQuery::Intersects:
      d->UI.crossEvent->setChecked(true);
      break;
    case vvDatabaseQuery::IntersectsInbound:
      d->UI.enterEvent->setChecked(true);
      break;
    case vvDatabaseQuery::IntersectsOutbound:
      d->UI.exitEvent->setChecked(true);
      break;
    default:
      d->UI.anyEvent->setChecked(true);
      break;
    }

  // Reset query ID (will have been cleared by setting UI controls to new query)
  d->query_.abstractQuery()->QueryId = query.abstractQuery()->QueryId;
}

//-----------------------------------------------------------------------------
void vqQueryDialog::setError(qtStatusSource, QString msg)
{
  QTE_D(vqQueryDialog);
  if (!d->error_.isEmpty())
    {
    d->error_ += '\n';
    }
  d->error_ += msg;
}

//-----------------------------------------------------------------------------
void vqQueryDialog::saveQuery()
{
  QTE_D(vqQueryDialog);

  d->fixupQuery();
  vvQueryInstance query = d->query_;

  if (!d->UI.regionLimit->isChecked())
    {
    query.abstractQuery()->SpatialFilter = vvDatabaseQuery::Ignore;
    }

  this->saveQuery(query, this);
}

//-----------------------------------------------------------------------------
void vqQueryDialog::saveQuery(const vvQueryInstance& query, QWidget* parent)
{
  QString selectedFilter;
  QString fileName = vgFileDialog::getSaveFileName(
                       parent, "Saved query...", QString(),
                       "VisGUI Query Plans (*.vqp *.vqpx);;"
                       "VisGUI Query Plans - KST (*.vqp);;"
                       "VisGUI Query Plans - XML (*.vqpx *.xml);;"
                       "All files (*)", &selectedFilter);
  if (fileName.isEmpty())
    {
    return;
    }

  // Open output file
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
    QString msg = "Unable to open file \"%1\" for writing: %2";
    QMessageBox::critical(parent, "Error writing file",
                          msg.arg(fileName).arg(file.errorString()));
    return;
    }

  // Choose format from extension
  const QString ext = QFileInfo(fileName).suffix();
  const bool saveAsXml =
    (ext == "xml" || ext == "vqpx" || selectedFilter.contains("XML"));
  vvWriter::Format format = (saveAsXml ? vvWriter::Xml : vvWriter::Kst);

  // Write the query plan
  vvWriter writer(file, format);
  writer << vvHeader::QueryPlan << query;
}

//-----------------------------------------------------------------------------
void vqQueryDialog::resetQueryId()
{
  QTE_D(vqQueryDialog);

  // Make the query ID empty, so we don't have to generate a new one every time
  // the user touches something; we'll generate an ID when it becomes needed
  d->query_.abstractQuery()->QueryId.clear();
}

//-----------------------------------------------------------------------------
void vqQueryDialog::updateTimeUpperFromLower()
{
  QTE_D(vqQueryDialog);
  const QDateTime value = d->UI.timeLower->dateTime();
  if (d->UI.timeUpper->dateTime() < value)
    d->UI.timeUpper->setDateTime(value);
}

//-----------------------------------------------------------------------------
void vqQueryDialog::updateTimeLowerFromUpper()
{
  QTE_D(vqQueryDialog);
  const QDateTime value = d->UI.timeUpper->dateTime();
  if (d->UI.timeLower->dateTime() > value)
    d->UI.timeLower->setDateTime(value);
}

//-----------------------------------------------------------------------------
void vqQueryDialog::requestRegion()
{
  QTE_D(vqQueryDialog);
  this->hide();
  connect(d->core_, SIGNAL(regionComplete(vgGeocodedPoly)),
          this, SLOT(acceptDrawnRegion(vgGeocodedPoly)));
  d->core_->drawRegion();
}

//-----------------------------------------------------------------------------
void vqQueryDialog::acceptDrawnRegion(vgGeocodedPoly region)
{
  QTE_D(vqQueryDialog);
  d->setQueryRegion(region);
  this->show();
}

//-----------------------------------------------------------------------------
void vqQueryDialog::editRegionNumeric()
{
  QTE_D(vqQueryDialog);

  vqRegionEditDialog erDialog;
  erDialog.setRegion(d->query_.abstractQuery()->SpatialLimit);

  if (erDialog.exec() == QDialog::Accepted)
    {
    d->setQueryRegion(erDialog.region());
    }
}

//END vqQueryDialog
