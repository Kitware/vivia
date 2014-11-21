/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
