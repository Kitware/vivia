// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqQueryDialog_h
#define __vqQueryDialog_h

#include <QDateTime>
#include <QDialog>

#include <qtStatusSource.h>

#include <vvQueryInstance.h>

class vqCore;

class vqQueryDialogPrivate;

class vqQueryDialog : public QDialog
{
  Q_OBJECT

public:
  vqQueryDialog(vqCore* core, bool useAdvancedUi,
                QWidget* parent = 0, Qt::WindowFlags flags = 0);
  virtual ~vqQueryDialog();

  void loadPersistentQuery();

  vvQueryInstance query();

  void initiateDatabaseQuery(std::string videoUri, long long startTimeLimit,
                             long long endTimeLimit, long long initialTime);

  static void saveQuery(const vvQueryInstance& query, QWidget* parent = 0);
  static void savePersistentQuery(const vvQueryInstance& query);

signals:
  void readyToProcessDatabaseVideoQuery(vvQueryInstance);

public slots:
  virtual void accept();

protected slots:
  void updateTimeUpperFromLower();
  void updateTimeLowerFromUpper();

  void preSetQueryType();
  void setQueryType(int);

  void requestRegion();
  void acceptDrawnRegion(vgGeocodedPoly region);

  void editRegionNumeric();

  void editQuery();

  void loadQuery();
  void saveQuery();

  void setQuery(vvQueryInstance, bool setType = true);
  void setError(qtStatusSource, QString);

  void resetQueryId();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vqQueryDialog)

private:
  QTE_DECLARE_PRIVATE(vqQueryDialog)
};

#endif
