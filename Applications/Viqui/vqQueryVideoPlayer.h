// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqQueryVideoPlayer_h
#define __vqQueryVideoPlayer_h

#include <QUrl>

#include <vvQueryVideoPlayer.h>

class vqQueryVideoPlayer : public vvQueryVideoPlayer
{
  Q_OBJECT

public:
  vqQueryVideoPlayer(QWidget* parent = 0);
  ~vqQueryVideoPlayer();

  bool setVideoUri(QUrl uri);

  virtual int videoHeight();

  void setTimeRange(double startTime, double endTime);

public slots:
  virtual void reset();

protected:
  virtual void buildVideoModel();

protected:
  struct qvpInternal;
  qvpInternal* MyInternal;
};

#endif
