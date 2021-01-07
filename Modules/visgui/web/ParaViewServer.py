# This file is part of ViViA, and is distributed under the
# OSI-approved BSD 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/vivia/blob/master/LICENSE for details.

# Import python core libraries
import sys
import os
import types

import argparse
import json

from PySide.QtCore import QCoreApplication, QUrl

import qt4reactor

QCoreApplication.setOrganizationName("Kitware")
QCoreApplication.setOrganizationDomain("kitware.com")

app = QCoreApplication(sys.argv)
qt4reactor.install()

# Import annotations
from autobahn.wamp import exportRpc

# Import paraview modules
from paraview import simple, paraviewweb_wamp, paraviewweb_protocols

# Currently ParaView does not load these modules
from vtkCommonCorePython import *
from vtkRenderingCorePython import *

# Import visgui
from vgVideoPython import *
from vtkVgCorePython import *
from vtkVwCorePython import *

from visgui.core import QueryService
from visgui.core import VideoPlayer

#==============================================================================
class WebProtocol(paraviewweb_wamp.ServerProtocol):
  authKey = "paraviewweb-secret"

  #----------------------------------------------------------------------------
  def initialize(self):
    """
    Called by the default server and serves as a demonstration purpose. This
    should be overridden by application protocols to set up the application
    specific pipeline.
    """
    self.registerParaViewWebProtocol(paraviewweb_protocols.ParaViewWebMouseHandler())
    self.registerParaViewWebProtocol(paraviewweb_protocols.ParaViewWebViewPort())
    self.registerParaViewWebProtocol(paraviewweb_protocols.ParaViewWebViewPortImageDelivery())
    self.registerParaViewWebProtocol(paraviewweb_protocols.ParaViewWebViewPortGeometryDelivery())
    self.registerParaViewWebProtocol(paraviewweb_protocols.ParaViewWebTimeHandler())

    # Update authentication key to use
    self.updateSecret(WebProtocol.authKey)

    # RPC objects
    self._queryService = None

    # RPC object collections
    self._videos = {}
    self._nextVideo = 0

    self._videoPlayers = {}
    self._nextVideoPlayer = 0

    # Other member variables
    self._view = None
    self._renWin = None
    self._view = simple.GetRenderView()
    self._view.OrientationAxesVisibility = False
    self._view.CenterAxesVisibility = False
    self._view.CameraParallelProjection = True
    self._view.InteractionMode = '2D'

    # !!!This ugly bit should go away if ParaView adds in sensible
    # default 2D interactor manipulators.
    pxm = simple.servermanager.ProxyManager()
    manipulator1 = pxm.NewProxy('cameramanipulators', 'TrackballPan1')
    manipulator1.GetProperty('Button').SetElement(0, 1) # left click
    manipulator1.UpdateVTKObjects()
    manipulator2 = pxm.NewProxy('cameramanipulators', 'TrackballZoom')
    manipulator2.GetProperty('Button').SetElement(0, 3) # right click
    manipulator2.UpdateVTKObjects()
    self._view.GetProperty('Camera2DManipulators').SetData([manipulator1, manipulator2])

    simple.Render()
    self._view.ViewSize = [800,800]
    self._renWin = self._view.GetRenderWindow()

    self._videoProvider = vgKwaArchive()
    providers = os.environ.get('VISGUI_VIDEO_PROVIDERS', None)
    if providers is not None:
      for provider in providers.split(';'):
        print 'add provider ' + provider
        self._videoProvider.addSource(provider)

  #----------------------------------------------------------------------------
  @exportRpc
  def getQueryUrl(self):
    return os.environ.get('VISGUI_QUERY_URL', '')

  #----------------------------------------------------------------------------
  @exportRpc
  def executeQuery(self, queryServiceUri, queryPlanData, workingSetSize):
    self._queryService = QueryService()
    return self._queryService.ExecuteQuery(QUrl(queryServiceUri),
                                           queryPlanData, workingSetSize)

  #----------------------------------------------------------------------------
  @exportRpc("query:getAllResults")
  def getAllResults(self, *args):
    return self._queryService.GetResultsAndFeedbackRequests(*args)

  #----------------------------------------------------------------------------
  @exportRpc
  def createVideo(self, videoUri):
    video = vtkVwVideo()
    if video.Open(videoUri):
      self._nextVideo += 1
      i = self._nextVideo
      self._videos[i] = video
      return i

  #----------------------------------------------------------------------------
  @exportRpc
  def createVideoPlayer(self):
    ren = self._view.GetRenderer()
    self._nextVideoPlayer += 1
    i = self._nextVideoPlayer
    self._videoPlayers[i] = VideoPlayer(ren)
    return i

  #----------------------------------------------------------------------------
  @exportRpc
  def setVideoStream(self, playerId, streamId):
    # For now use the same video stream
    # @todo Remove this code
    streamId = os.environ.get('VISGUI_STREAM_ID', None)
    if streamId is None:
      raise EnvironmentError(0, 'VISGUI_STREAM_ID is not set')
    if playerId in self._videoPlayers:
      retval = self._videoPlayers[playerId].Init(streamId)
    else:
      retval = -1
    return retval

  #----------------------------------------------------------------------------
  @exportRpc
  def setVideoData(self, playerId, resultId):
    if not playerId in self._videoPlayers:
      return 'Video player %s not found' % playerId

    videoPlayer = self._videoPlayers[playerId]
    result = self._queryService.GetSerializedResult(resultId)

    try:
      if 'streamId' in result:
        request = vgKwaArchiveRequest()
        request.StreamId = result['streamId']
        if 'missionId' in result and not result['missionId'] == '-undefined-':
          request.MissionId = result['missionId']
        request.StartTime = result['startTime']
        request.EndTime = result['endTime']
        # TODO padding

        uri = self._videoProvider.getUri(request).toString()
        self._view.ResetCamera()
        videoPlayer.Init(uri)

      if 'descriptors' in result:
        descriptors = result['descriptors']
        tid = 0
        for descriptor in descriptors:
          if 'regions' in descriptor:
            track = vtkVgTrack()
            videoPlayer.Stop()
            videoPlayer.InitializeTrack(track)
            tid += 1
            track.SetId(tid)

            for region in descriptor['regions']:
              ts = vtkVgTimeStamp()
              jsts = region['timeStamp']
              if 'frame' in jsts:
                ts.SetFrameNumber(jsts['frame'])
              if 'time' in jsts:
                ts.SetTime(jsts['time'])
              ir = region['imageRegion']
              left = ir['left']
              right = ir['right']
              # TODO Assuming video is height==480 for now... We really should
              #      be using the actual video meta data for both the height
              #      and to use the homographies for stabilization, but not
              #      sure that is going to be done in the Python layer.
              top = 480 - ir['top']
              bottom = 480 - ir['bottom']

              shell = vtkDenseArray[float]()
              shell.Resize(12)

              def setShellPoint(i, x, y):
                i *= 3
                shell.SetValue(i + 0, x)
                shell.SetValue(i + 1, y)
                shell.SetValue(i + 2, 0.0)

              setShellPoint(0, left, top)
              setShellPoint(1, right, top)
              setShellPoint(2, right, bottom)
              setShellPoint(3, left, bottom)

              track.InsertNextPoint(ts, [0.5 * (left + right), bottom],
                                    vtkVgGeoCoord(), shell)

          videoPlayer.AddTrack(track)

        videoPlayer.Update()

    except:
      print 'error ', sys.exc_info()[1]

  #----------------------------------------------------------------------------
  @exportRpc
  def render(self):
    simple.Render()

#==============================================================================
def bindRpc(cls, ns, name, mctor):
  fullName = name
  rpcName = name

  if ns is not None:
    rpcName = ns + ':' + name
    fullName = '__' + ns + '_' + name

  rpc = types.MethodType(exportRpc(rpcName)(mctor(fullName)), None, cls)
  setattr(cls, fullName, rpc)

#==============================================================================
def addRpc(cls, obj, ns, call):
  call_uc = call[0].upper() + call[1:]

  def createRpc(call):
    def dispatchRpc(self, *args, **kwargs):
      return getattr(getattr(self, obj), call_uc)(*args, **kwargs)
    return dispatchRpc

  bindRpc(cls, ns, call, createRpc)

#==============================================================================
def addMappedRpc(cls, objs, ns, call):
  call_uc = call[0].upper() + call[1:]

  def createRpc(call):
    def dispatchRpc(self, i, *args, **kwargs):
      c = getattr(self, objs)
      if i in c:
        return getattr(c[i], call_uc)(*args, **kwargs)
    return dispatchRpc

  bindRpc(cls, ns, call, createRpc)

#==============================================================================
def addArguments(parser):
  """
  Add arguments processed know to this module. parser must be
  argparse.ArgumentParser instance.
  """
  parser.add_argument("-d", "--debug",
    help="log debugging messages to stdout",
    action="store_true")
  parser.add_argument("-p", "--port", type=int, default=8080,
    help="port number on which the server will listen (default: 8080)")
  parser.add_argument("-t", "--timeout", type=int, default=300,
    help="timeout for reaping process on idle in seconds (default: 300s)")
  parser.add_argument("-c", "--content", default=None,
    help="root path of common web content to serve")
  parser.add_argument("-a", "--app-content", default=None, dest='app_content',
    help="root path of application-specific web content to serve")
  parser.add_argument("-k", "--authKey", default=WebProtocol.authKey,
    help="authentication key for clients to connect to the web socket")

  return parser

#==============================================================================
def startServer(options, disableLogging=False):
  """
  Starts the web server. Options must be an object with the following members:
    options.port : port number on which the server will listen
    options.timeout : timeout (seconds) for reaping process on idle
    options.content : root path of common content to serve
    options.app_content : root path of application-specific content to serve
  """
  from twisted.python import log
  from twisted.internet import reactor
  from twisted.web.server import Site
  from twisted.web.static import File

  from autobahn.resource import WebSocketResource
  from paraview.webgl import WebGLResource

  if options.content is None and options.app_content is None:
    raise EnvironmentError(0, 'No content specified')

  if not disableLogging:
    log.startLogging(sys.stdout)

  # Set up the server factory
  wampFactory = paraviewweb_wamp.ReapingWampServerFactory(
    "ws://localhost:%d" % options.port, options.debug, options.timeout)
  wampFactory.protocol = WebProtocol

  # Set up the site
  root = None
  if options.app_content is None:
    root = File(options.content)
  elif options.content is None:
    root = File(options.app_content)
  else:
    root = File(options.content)
    root.putChild("app", File(options.app_content))

  root.putChild("ws", WebSocketResource(wampFactory))
  root.putChild("WebGL", WebGLResource());

  # Start factory and reactor
  wampFactory.startFactory()
  reactor.listenTCP(options.port, Site(root))
  reactor.run()
  wampFactory.stopFactory()

#==============================================================================
def run():
  # Parse arguments
  parser = argparse.ArgumentParser(
    description="ParaView/Web file loader web-application")
  addArguments(parser)
  args = parser.parse_args()

  # Add dynamically-created RPC's
  for rpc in ("shutDown", "isExecuting", "isCompleted", "getResultCount",
              "getStatus", "setResultFeedback", "refine"):
    addRpc(WebProtocol, "_queryService", "query", rpc)

  for rpc in ("getFrame", "getFirstTime", "getLastTime", "getFrameCount"):
    addMappedRpc(WebProtocol, "_videos", "video", rpc)

  for rpc in ("play", "stop", "seek", "setTrailsVisible"):
    addMappedRpc(WebProtocol, "_videoPlayers", "videoPlayer", rpc)

  # Set up an start the web server
  # Commented out for now since we don't need to do rendering
  WebProtocol.authKey = args.authKey
  startServer(options=args)

#==============================================================================
if __name__ == '__main__':
  run()
