// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "moc_vqPredefinedQueryCachePrivate.cpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>

#include <qtOverlayWidget.h>
#include <qtThrobber.h>

#include "vqQueryParser.h"
#include "vqScopedOverrideCursor.h"
#include "vqSettings.h"

QTE_IMPLEMENT_D_FUNC(vqPredefinedQueryCache)

//-----------------------------------------------------------------------------
class vqPredefinedQueryCacheWaitDialog : public QDialog
{
public:
  explicit vqPredefinedQueryCacheWaitDialog(
    QWidget* parent = 0, Qt::WindowFlags f = 0) : QDialog(parent, f) {}
  virtual ~vqPredefinedQueryCacheWaitDialog() {}

  virtual void reject() { /* disallow */ }
};

//-----------------------------------------------------------------------------
vqPredefinedQueryCachePrivate::vqPredefinedQueryCachePrivate()
  : PlansReady(false), Running(false), Interrupt(false)
{
  QTE_REGISTER_METATYPE(vqPredefinedQueryList);
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryCachePrivate::run()
{
  QString path = vqSettings().predefinedQueryUri().toLocalFile();
  if (path.isEmpty())
    {
    // No plans available; return immediately
    emit this->loadingComplete(vqPredefinedQueryList());
    return;
    }

  QDir dir(path);

  QStringList filters;
  filters << "*.vqp";
  dir.setNameFilters(filters);
  dir.setFilter(QDir::Readable | QDir::Files);

  QStringList files = dir.entryList();

  // Loop over files
  foreach (QString fileName, files)
    {
    if (this->Interrupt)
      {
      // We were interrupted; die immediately (parent will be waiting on our
      // thread, not a signal)
      return;
      }

    if (fileName.startsWith('.'))
      {
      // Ignore hidden files
      continue;
      }

    // Load the query plan
    this->loadQuery(path, fileName);
    }

  // Done
  emit this->loadingComplete(this->LoadedPlans);
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryCachePrivate::loadQuery(
  const QString& path, const QString& fileName)
{
  this->CurrentlyLoadingPlan = fileName;
  const QString filePath = QString("%1/%2").arg(path, fileName);

  // Set up query parser to load query
  vqQueryParser parser;
  connect(&parser, SIGNAL(planAvailable(vvQueryInstance)),
          this, SLOT(acceptLoadedQuery(vvQueryInstance)),
          Qt::QueuedConnection);
  connect(&parser, SIGNAL(error(qtStatusSource, QString)),
          this, SLOT(abortQueryLoading(qtStatusSource, QString)),
          Qt::QueuedConnection);

  // Fire off query load, and wait on parser to finish
  parser.loadQuery(QUrl::fromLocalFile(filePath));
  qtThread::run();
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryCachePrivate::acceptLoadedQuery(vvQueryInstance query)
{
  // For the moment at least, predefined retrieval queries don't really make
  // sense, since they don't have any options we would want to pass along (i.e.
  // they are not interesting compared to just selecting the respective query
  // type)... and also, they have no descriptors, which would work out rather
  // poorly given that our user, vqPredefinedQueryDialog, is a subclass of
  // vvAbstractSimilarityQueryDialog... so we only accept similarity queries
  if (query.isSimilarityQuery())
    {
    // Add query to internal list
    const QFileInfo fi(this->CurrentlyLoadingPlan);
    this->LoadedPlans.insert(fi.completeBaseName(), query);
    }

  // Interrupt wait loop to continue to next file
  this->quit();
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryCachePrivate::abortQueryLoading(
  qtStatusSource, QString error)
{
  qDebug() << "vqPredefinedQueryCache: Error loading query from"
           << this->CurrentlyLoadingPlan << '-' << qPrintable(error);
  this->quit();
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryCachePrivate::reload()
{
  vqScopedOverrideCursor oc(Qt::BusyCursor);

  // Interrupt running thread (if any)
  this->Interrupt = true;
  this->wait();

  // Reset and reload
  this->Interrupt = false;
  this->PlansReady = false;
  this->start();
  this->Running = true;
}

//-----------------------------------------------------------------------------
vqPredefinedQueryList vqPredefinedQueryCachePrivate::getAvailableQueryPlans()
{
  // Wait for plans to become available
  if (!this->PlansReady)
    {
    vqPredefinedQueryCacheWaitDialog waitDialog;
    this->WaitDialog = &waitDialog;

    qtOverlayWidget* widget = new qtOverlayWidget(&waitDialog);

    qtThrobber* throbber = new qtThrobber(widget);
    throbber->setMinimumSize(64, 64);
    throbber->setActive(true);

    QLabel* label = new QLabel(widget);
    label->setText("Loading query plans;\nPlease be patient...");

    widget->addWidget(throbber);
    widget->addWidget(label);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(widget);

    waitDialog.setLayout(layout);
    waitDialog.resize(layout->minimumSize());

    if (!this->Running)
      {
      // If for some reason we are not already loading the plans, do so now
      this->reload();
      }

    // Show wait dialog until plans are ready
    waitDialog.exec();
    }

  return this->AvailableQueryPlans;
}

//-----------------------------------------------------------------------------
vqPredefinedQueryCache* vqPredefinedQueryCache::instance()
{
  static vqPredefinedQueryCache* theInstance = 0;
  if (!theInstance)
    {
    // If our singleton has not yet been created, create it now, parented to
    // the QApplication so that it will get cleaned up
    theInstance = new vqPredefinedQueryCache(qApp);
    }
  return theInstance;
}

//-----------------------------------------------------------------------------
vqPredefinedQueryCache::vqPredefinedQueryCache(QObject* parent)
  : QObject(parent), d_ptr(new vqPredefinedQueryCachePrivate)
{
  QTE_D(vqPredefinedQueryCache);
  connect(d, SIGNAL(loadingComplete(vqPredefinedQueryList)),
          this, SLOT(setAvailableQueryPlans(vqPredefinedQueryList)));
}

//-----------------------------------------------------------------------------
vqPredefinedQueryCache::~vqPredefinedQueryCache()
{
  QTE_D(vqPredefinedQueryCache);
  d->Interrupt = true;
  d->quit();
  d->wait();
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryCache::reload()
{
  vqPredefinedQueryCache::instance()->d_func()->reload();
}

//-----------------------------------------------------------------------------
vqPredefinedQueryList vqPredefinedQueryCache::getAvailableQueryPlans()
{
  return vqPredefinedQueryCache::instance()->d_func()->getAvailableQueryPlans();
}

//-----------------------------------------------------------------------------
void vqPredefinedQueryCache::setAvailableQueryPlans(
  vqPredefinedQueryList plans)
{
  QTE_D(vqPredefinedQueryCache);
  d->PlansReady = true;
  d->AvailableQueryPlans = plans;
  d->wait();
  d->Running = false;

  if (d->WaitDialog)
    {
    // We're done; close wait dialog
    d->WaitDialog.data()->accept();
    }
}
