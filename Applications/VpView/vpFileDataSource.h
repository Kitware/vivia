/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
