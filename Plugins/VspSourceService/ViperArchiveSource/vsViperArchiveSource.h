// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsViperArchiveSource_h
#define __vsViperArchiveSource_h

#include <QUrl>

#include <vsSourceFactory.h>
#include <vsDescriptorSource.h>

class vsViperArchiveSourcePrivate;

class vsViperArchiveSource : public vsDescriptorSource
{
  Q_OBJECT

public:
  explicit vsViperArchiveSource(const QUrl& archiveUri);
  virtual ~vsViperArchiveSource();

  virtual void start();

  virtual vsDescriptorInput::Types inputAccepted() const;
  virtual void injectInput(qint64 id, vsDescriptorInputPtr input);

  virtual Status status() const;
  virtual QString text() const;
  virtual QString toolTip() const;

protected slots:
  void updateStatus(vsDataSource::Status);

  void getImportOptions();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsViperArchiveSource)

  virtual vsTrackSourcePtr trackSource();

private:
  QTE_DECLARE_PRIVATE(vsViperArchiveSource)
  QTE_DISABLE_COPY(vsViperArchiveSource)
  friend class vsViperArchiveSourcePlugin;
};

#endif
