/*ckwg +5
 * Copyright 2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsVvqsDatabaseQueryDialog.h"
#include "ui_vvqsDatabaseQuery.h"

#include <vvQueryServerDialog.h>
#include <vvQueryService.h>

#include <vgUnixTime.h>

#include <qtUiState.h>
#include <qtUtil.h>

#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QUrlQuery>

QTE_IMPLEMENT_D_FUNC(vsVvqsDatabaseQueryDialog)

//-----------------------------------------------------------------------------
class vsVvqsDatabaseQueryDialogPrivate
{
public:
  Ui::vsVvqsDatabaseQueryDialog UI;
  qtUiState UiState;
  QUrl Uri;
};

//-----------------------------------------------------------------------------
vsVvqsDatabaseQueryDialog::vsVvqsDatabaseQueryDialog(
  QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags), d_ptr(new vsVvqsDatabaseQueryDialogPrivate)
{
  QTE_D(vsVvqsDatabaseQueryDialog);
  d->UI.setupUi(this);
  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Restore previous request
  d->UiState.setCurrentGroup("Database");
  d->UiState.mapText("Server", d->UI.serverUri);
  d->UiState.mapText("StreamLimit", d->UI.streamId);
  d->UiState.mapChecked("TemporalLimit", d->UI.timeLimit);
  d->UiState.mapChecked("ExtractClassifiers", d->UI.extractClassifiers);
  d->UiState.restore();
  if (d->UI.timeLimit->isChecked())
    {
    QSettings settings;
    settings.beginGroup("Database");
    const long long tl = settings.value("TemporalLower", 0).toLongLong();
    const long long tu = settings.value("TemporalUpper", 0).toLongLong();
    d->UI.timeLower->setDateTime(vgUnixTime(tl).toDateTime().toUTC());
    d->UI.timeUpper->setDateTime(vgUnixTime(tu).toDateTime().toUTC());
    }

  // Encourage user to provide a stream limit if no limit is set
  d->UI.streamLimit->setChecked(!d->UI.streamId->text().isEmpty() ||
                                !d->UI.timeLimit->isChecked());

  // Set default min/max values for temporal limit
  QDateTime epoch = QDateTime::fromMSecsSinceEpoch(0).toUTC();
  d->UI.timeLower->setMinimumDateTime(epoch.addYears(-40));
  d->UI.timeUpper->setMinimumDateTime(epoch.addYears(-40));
  d->UI.timeLower->setMaximumDateTime(epoch.addYears(530).addSecs(-1));
  d->UI.timeUpper->setMaximumDateTime(epoch.addYears(530).addSecs(-1));

  // Connect UI signals
  connect(d->UI.serverUriEdit, SIGNAL(clicked(bool)),
          this, SLOT(editServerUri()));
  connect(d->UI.timeLower, SIGNAL(editingFinished()),
          this, SLOT(updateTimeUpperFromLower()));
  connect(d->UI.timeUpper, SIGNAL(editingFinished()),
          this, SLOT(updateTimeLowerFromUpper()));

  connect(d->UI.serverUri, SIGNAL(textChanged(QString)),
          this, SLOT(validate()));
  connect(d->UI.streamId, SIGNAL(textChanged(QString)),
          this, SLOT(validate()));
  connect(d->UI.streamLimit, SIGNAL(toggled(bool)),
          this, SLOT(validate()));

  this->validate();
}

//-----------------------------------------------------------------------------
vsVvqsDatabaseQueryDialog::~vsVvqsDatabaseQueryDialog()
{
}

//-----------------------------------------------------------------------------
QUrl vsVvqsDatabaseQueryDialog::uri() const
{
  QTE_D_CONST(vsVvqsDatabaseQueryDialog);
  return d->Uri;
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseQueryDialog::accept()
{
  QTE_D(vsVvqsDatabaseQueryDialog);

  if (!d->UI.streamLimit->isChecked() && !d->UI.timeLimit->isChecked())
    {
    // No limits are defined; this is probably not good, so require user
    // confirmation before proceeding anyway
    const char* msg =
      "Your request does not have any limits defined. If you continue with "
      "this request, the entire contents of the database will be retrieved. "
      "This may take a considerable amount of time and lead to application "
      "instability, and is probably not what you want.\n\nAre you sure you "
      "want to continue?";
    QMessageBox::StandardButton result =
      QMessageBox::warning(this, "Are you sure?", msg,
                           QMessageBox::Yes | QMessageBox::No);

    if (result != QMessageBox::Yes)
      {
      // User not sure; do nothing (return to dialog and allow user to make
      // changes)
      return;
      }
    }

  // Form request URI and save options for next time... these are interleaved
  // because both behave differently depending on if a temporal limit has been
  // requested
  QSettings settings;
  settings.beginGroup("Database");
  d->Uri = QUrl::fromUserInput(d->UI.serverUri->text());
  d->UiState.save();

  auto query = QUrlQuery{d->Uri};

  if (d->UI.streamLimit->isChecked())
    {
    query.addQueryItem("Stream", d->UI.streamId->text());
    }
  else
    {
    d->UI.streamId->clear();
    }

  const bool timeLimit = d->UI.timeLimit->isChecked();
  if (timeLimit)
    {
    const vgUnixTime tl(d->UI.timeLower->dateTime());
    const vgUnixTime tu(d->UI.timeUpper->dateTime());
    query.addQueryItem("TemporalLower", QString::number(tl.toInt64()));
    query.addQueryItem("TemporalUpper", QString::number(tu.toInt64()));
    settings.setValue("TemporalLower", tl.toInt64());
    settings.setValue("TemporalUpper", tu.toInt64());
    }

  if (d->UI.extractClassifiers->isChecked())
    {
    query.addQueryItem("ExtractClassifiers", "true");
    }

  d->Uri.setQuery(query);

  // Done
  QDialog::accept();
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseQueryDialog::updateTimeUpperFromLower()
{
  QTE_D(vsVvqsDatabaseQueryDialog);
  const QDateTime value = d->UI.timeLower->dateTime();
  if (d->UI.timeUpper->dateTime() < value)
    {
    d->UI.timeUpper->setDateTime(value);
    }
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseQueryDialog::updateTimeLowerFromUpper()
{
  QTE_D(vsVvqsDatabaseQueryDialog);
  const QDateTime value = d->UI.timeUpper->dateTime();
  if (d->UI.timeLower->dateTime() > value)
    {
    d->UI.timeLower->setDateTime(value);
    }
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseQueryDialog::editServerUri()
{
  QTE_D(vsVvqsDatabaseQueryDialog);

  vvQueryServerDialog dialog;
  vvQueryService::registerChoosers(&dialog);
  int result = dialog.exec(QUrl::fromUserInput(d->UI.serverUri->text()));
  if (result == QDialog::Accepted)
    {
    d->UI.serverUri->setText(dialog.uri().toString());
    }
}

//-----------------------------------------------------------------------------
void vsVvqsDatabaseQueryDialog::validate()
{
  QTE_D(vsVvqsDatabaseQueryDialog);
  bool valid = true;

  valid = valid && !d->UI.serverUri->text().isEmpty();
  valid = valid && !(d->UI.streamLimit->isChecked() &&
                     d->UI.streamId->text().isEmpty());

  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}
