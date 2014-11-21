/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsQfVideoPlayer_h
#define __vsQfVideoPlayer_h

#include <QUrl>

#include <vvQueryVideoPlayer.h>

class vsQfVideoSource;

struct vtkVgVideoMetadata;

class vsQfVideoPlayer : public vvQueryVideoPlayer
{
  Q_OBJECT

public:
  vsQfVideoPlayer(vsQfVideoSource* source, QWidget* parent = 0);
  ~vsQfVideoPlayer();

  virtual int videoHeight();

  vsQfVideoSource* source() { return this->Source; }

public slots:
  virtual void reset();

protected:
  virtual void buildVideoModel();

private:
  vsQfVideoSource* Source;
};

#endif
