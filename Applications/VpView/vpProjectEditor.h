// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
  void setDataset(const QString&);

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
