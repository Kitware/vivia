// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
