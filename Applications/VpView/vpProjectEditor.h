/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpProjectEditor_h
#define __vpProjectEditor_h

#include <QDialog>

#include <qtGlobal.h>

class vpProjectEditorPrivate;

class vpProjectEditor : public QDialog
{
  Q_OBJECT

public:
  vpProjectEditor(QWidget* parent = nullptr, Qt::WindowFlags flags = {});
  ~vpProjectEditor();

  QString projectPath() const;

public slots:
  void saveProject(const QString&);

protected slots:
  void browseForDatasetFile();
  void browseForDatasetDirectory();

  void browseForTracksFile();

  void saveProject();
  void acceptProject();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpProjectEditor)

private:
  QTE_DECLARE_PRIVATE(vpProjectEditor)
};

#endif
