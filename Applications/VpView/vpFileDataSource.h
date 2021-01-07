// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpFileDataSource_h
#define __vpFileDataSource_h

#include <qtGlobal.h>

#include <QObject>

class vpFileDataSourcePrivate;

class vpFileDataSource : public QObject
{
  Q_OBJECT

public:
  vpFileDataSource();
  ~vpFileDataSource();

  void setDataSetSpecifier(const QString& source);
  QString dataSetSpecifier() const;

  void setDataFiles(const QStringList& filenames);

  int frames() const;
  QString frameName(int index) const;
  QStringList frameNames() const;

  void setMonitoringEnabled(bool);

signals:
  void frameSetChanged();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpFileDataSource)

  void update();

protected slots:
  void updateDataFiles();

private:
  QTE_DECLARE_PRIVATE(vpFileDataSource)
};

#endif
