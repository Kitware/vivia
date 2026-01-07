// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqConfigureDialog_h
#define __vqConfigureDialog_h

#include <QDialog>

#include <vvDescriptorStyle.h>
#include <vvScoreGradient.h>

#include "vqSettings.h"

#include "ui_configure.h"

class vqConfigureDialog : public QDialog
{
  Q_OBJECT

public:
  vqConfigureDialog(QWidget* parent);
  virtual ~vqConfigureDialog();

  Ui::ConfigureDialog UI;

signals:
  void scoreGradientChanged();
  void predefinedQueryUriChanged();

public slots:
  void accept();
  void reject();
  void apply();
  void reset();

protected slots:
  void queryServerUriChanged(QString newUri);
  void queryVideoUriChanged(QString newUri);
  void queryCacheUriChanged(QString newUri);
  void predefinedQueryUriChanged(QString newUri);
  void showQueryServerEditUriDialog();
  void showQueryVideoPickLocalDirDialog();
  void showQueryCachePickLocalDirDialog();
  void showVideoProviderAddLocalArchiveDialog();
  void showPredefinedQueryPickLocalDirDialog();
  void updateVideoProviderButtons();
  void videoProvidersSelectionMoveUp();
  void videoProvidersSelectionMoveDown();
  void videoProvidersSelectionDelete();
  void resultsPerPageChanged(int newValue);
  void resultClipPaddingChanged(double newValue);
  void resultColoringChanged(vvScoreGradient newGradient);
  void iqrWorkingSetSizeChanged(int newSize);
  void iqrRefinementSetSizeChanged(int newSize);
  void updateDescriptorButtons();
  void descriptorsChanged();
  void addDescriptor();
  void removeDescriptors();

protected:
  void setModified();
  QList<QUrl> videoProviders();
  vvDescriptorStyle::Map descriptorStyles();

  vqSettings* settings_;
};

#endif
