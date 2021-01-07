// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsDataSource_h
#define __vsDataSource_h

#include <QObject>

#include <qtGlobal.h>

#include <vgExport.h>

class VSP_DATA_EXPORT vsDataSource : public QObject
{
  Q_OBJECT

public:
  enum Status
    {
    NoSource                            = 0x000,
    StreamingPending                    = 0x101,
    StreamingIdle                       = 0x102,
    StreamingActive                     = 0x103,
    StreamingStopped                    = 0x100,
    InProcessIdle                       = 0x200,
    InProcessActive                     = 0x203,
    ArchivedIdle                        = 0x400,
    ArchivedActive                      = 0x403,
    ArchivedSuspended                   = 0x402,
    MultipleSources                     = 0x800,
    MultipleSourcesStreaming            = 0x900,
    MultipleSourcesInProcess            = 0xA00,
    MultipleSourcesStreamingInProcess   = 0xB00,
    MultipleSourcesArchived             = 0xC00,
    MultipleSourcesStreamingArchived    = 0xD00,
    MultipleSourcesInProcessArchived    = 0xE00,
    MultipleSourcesAll                  = 0xF00
    };

  static const int SourceTypeMask       = 0x700;

  virtual ~vsDataSource();

  virtual void start();

  virtual Status status() const;
  virtual QString text() const;
  virtual QString toolTip() const;

signals:
  void statusChanged(vsDataSource::Status);

protected:
  vsDataSource();

private:
  QTE_DISABLE_COPY(vsDataSource)
};

#endif
