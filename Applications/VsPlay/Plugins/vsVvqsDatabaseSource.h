// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsVvqsDatabaseSource_h
#define __vsVvqsDatabaseSource_h

#include <QUrl>

#include <vsSourceFactory.h>
#include <vsDescriptorSource.h>

class vvQuerySession;

class vsVvqsDatabaseSourcePrivate;

class vsVvqsDatabaseSource : public vsDescriptorSource
{
  Q_OBJECT

public:
  explicit vsVvqsDatabaseSource(vvQuerySession*, const QUrl& request);
  virtual ~vsVvqsDatabaseSource();

  virtual void start() QTE_OVERRIDE;

  virtual Status status() const QTE_OVERRIDE;
  virtual QString text() const QTE_OVERRIDE;
  virtual QString toolTip() const QTE_OVERRIDE;

protected slots:
  void updateStatus(vsDataSource::Status trackStatus,
                    vsDataSource::Status descriptorStatus);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsVvqsDatabaseSource)

  vsTrackSourcePtr trackSource();

private:
  QTE_DECLARE_PRIVATE(vsVvqsDatabaseSource)
  QTE_DISABLE_COPY(vsVvqsDatabaseSource)
  friend class vsVvqsDatabaseFactory;
};

#endif
