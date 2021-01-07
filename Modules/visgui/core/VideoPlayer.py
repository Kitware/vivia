# This file is part of ViViA, and is distributed under the
# OSI-approved BSD 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/vivia/blob/master/LICENSE for details.

from vtkVgCorePython import *
from vtkVgModelViewPython import *
from vtkVgVideoPython import *

import vtkRenderingCorePython

import sys
import time
from datetime import datetime

#==============================================================================
class VideoPlayer:
  #----------------------------------------------------------------------------
  def __init__(self, renderer):
    self._streamId = None
    self._renderer = renderer
    self._actor = vtkRenderingCorePython.vtkImageActor()
    self._videoSource = None
    self._videoModel = vtkVgVideoModel()
    self._trackModel = vtkVgTrackModel()
    self._trackModel.SetDisplayAllTracks(True)
    self._trackModel.SetTrackExpirationOffset(vtkVgTimeStamp(1.5e6, 15))
    self._trackModel.Initialize()
    self._videoModel.AddChildModel(self._trackModel)
    self._trackRepresentation = vtkVgTrackRepresentation()
    self._trackRepresentation.SetTrackModel(self._trackModel)
    self._trackRepresentation.SetVisible(True)
    self._trackRepresentation.SetZOffset(0.1)
    self._trackRepresentation.UseAutoUpdateOff()
    self._trackHeadRepresentation = vtkVgTrackHeadRepresentation()
    self._trackHeadRepresentation.SetTrackModel(self._trackModel)
    self._trackHeadRepresentation.SetVisible(True)
    self._trackHeadRepresentation.SetZOffset(0.2)
    self._trackHeadRepresentation.UseAutoUpdateOff()
    self._beginTime = None
    self._endTime = None
    self._lastFrameTimestamp = None
    self._currTimestamp = vtkVgTimeStamp()
    self._firstTime = None
    self._prevTime = None
    self._currTime = None
    self._frameData = None

  #----------------------------------------------------------------------------
  def Init(self, streamId):
    self._streamId = streamId
    self._videoSource = vtkVgKwaVideoSource()
    self._videoModel.SetVideoSource(self._videoSource)

    if not self._videoSource.Open(streamId):
      raise IOError(0, 'Unable to open video stream %s' % streamId)

    self._beginTime = self._videoSource.GetMinTime()
    self._endTime = self._videoSource.GetMaxTime()

    self._renderer.AddActor(self._actor)
    self._currTimestamp = self._beginTime
    self.Update()

    # Zoom to video frame extents
    frameImage = self._frameData.GetImageData()
    vtkVgRendererUtils().ZoomToImageExtents2D(self._renderer, frameImage)
    self.Update()

    # Remove any tracks that may exist from a previous video
    oldTracks = []
    self._trackModel.InitTrackTraversal()
    ti = self._trackModel.GetNextTrack()
    while ti.GetTrack():
      oldTracks.append(ti.GetTrack().GetId())
      ti = self._trackModel.GetNextTrack()

    for track in oldTracks:
      self._trackModel.RemoveTrack(track)

    return 1

  #----------------------------------------------------------------------------
  def UpdateRepresentation(self, rep):
    rep.Update()

    # Add new props
    props = rep.GetNewRenderObjects()
    props.InitTraversal()
    prop = props.GetNextProp()
    while prop is not None:
      self._renderer.AddViewProp(prop)
      prop = props.GetNextProp()

    # Remove expired props
    props = rep.GetExpiredRenderObjects();
    props.InitTraversal()
    prop = props.GetNextProp()
    while prop is not None:
      self._renderer.RemoveViewProp(prop)
      prop = props.GetNextProp()

    rep.ResetTemporaryRenderObjects();

  #----------------------------------------------------------------------------
  def Step(self, timestamp):
    self._videoModel.Update(timestamp)

    # Update track representations
    self.UpdateRepresentation(self._trackRepresentation)
    self.UpdateRepresentation(self._trackHeadRepresentation)

    # Update video 'representation'
    self._frameData = self._videoModel.GetCurrentVideoFrame()
    self._actor.SetInputData(self._frameData.GetImageData())
    self._actor.Update()

  #----------------------------------------------------------------------------
  def Play(self):
    if self._firstTime is None:
      self._firstTime = datetime.now()

    delta = (datetime.now() - self._firstTime)
    delta = (delta.days * 24 * 60 * 60 + delta.seconds) * 1e6 + delta.microseconds
    self._currTime =  self._beginTime.GetTime() + delta
    self._currTimestamp = vtkVgTimeStamp(self._currTime)
    self.Step(self._currTimestamp)
    return self._currTime

  #----------------------------------------------------------------------------
  def Seek(self, timestamp):
    # TODO handle in-play condition by checking self._firstTime
    self._currTime = float(timestamp)
    self._currTimestamp = vtkVgTimeStamp(self._currTime)
    self.Step(self._currTimestamp)
    return self._currTime

  #----------------------------------------------------------------------------
  def Stop(self):
    self._firstTime = None

  #----------------------------------------------------------------------------
  def SetTrailsVisible(self, state):
    self._trackRepresentation.SetVisible(state)

  #----------------------------------------------------------------------------
  def AddTrack(self, track):
    self._trackModel.AddTrack(track)

  #----------------------------------------------------------------------------
  def InitializeTrack(self, track):
    track.SetPoints(self._trackModel.GetPoints())

  #----------------------------------------------------------------------------
  def Update(self):
    self.Step(self._currTimestamp)

#==============================================================================
if __name__ == '__main__':
  if len(sys.argv) != 2:
    sys.exit("Usage: VideoPlayer filename_uri")

  ren = vtkRenderingCorePython.vtkRenderer()

  renWin = vtkRenderingCorePython.vtkRenderWindow()
  renWin.AddRenderer(ren)

  viewer = vtkRenderingCorePython.vtkRenderWindowInteractor()
  viewer.SetRenderWindow(renWin)
  cam = ren.GetActiveCamera()
  cam.ParallelProjectionOn()

  player = VideoPlayer(ren)
  player.Init(sys.argv[1])

  renWin.Render()
  viewer.Start()
