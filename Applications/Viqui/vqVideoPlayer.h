/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqVideoPlayer_h
#define __vqVideoPlayer_h

#include <QUrl>

#include <vvQueryFormulation.h>

#include <vvVideoPlayer.h>

class vtkVgVideoNode;
class vtkVgVideoRepresentationBase0;

class vqVideoPlayerPrivate;

class vqVideoPlayer : public vvVideoPlayer
{
  Q_OBJECT

public:
  explicit vqVideoPlayer(QWidget* parent = 0);
  ~vqVideoPlayer();

signals:
  void queryFormulationRequested(vvProcessingRequest, long long initialTime);
  void externalOpenRequested(QUrl clipUri, QString streamId, double time);

public slots:
  void setAnalysisToolsEnabled(bool);

protected slots:
  virtual void formulateQuery();
  virtual void openExternally();

protected:
  explicit vqVideoPlayer(vqVideoPlayerPrivate*, QWidget* parent = 0);
  virtual vtkVgVideoRepresentationBase0* buildVideoRepresentation(
    vtkVgVideoNode& videoNode);

  virtual void forceUpdateVideoRepresentation();

private:
  QTE_DECLARE_PRIVATE(vqVideoPlayer)
};

#endif // __vqVideoPlayer_h
