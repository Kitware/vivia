import argparse
import os
import sys
import unittest

from vtkImagingCorePython import *
from vtkIOImagePython import *
from vtkRenderingCorePython import *
import vtkRenderingOpenGLPython

#------------------------------------------------------------------------------
def className(obj):
  name = str(obj.__class__)
  if name[0] == '<':
    name = name[name.find('\'')+1:-2]
  return name[name.rfind('.')+1:]

#==============================================================================
class RenderContext(object):
  pass

#==============================================================================
class VtkTest(unittest.TestCase):
  #------------------------------------------------------------------------------
  @staticmethod
  def main():
    mainModule = sys.modules['__main__']
    tests = unittest.defaultTestLoader.loadTestsFromModule(mainModule)
    suite = unittest.TestSuite()
    suite.addTests(tests)

    result = unittest.TextTestRunner().run(suite)
    sys.exit(0 if result.wasSuccessful() else 1)

  #----------------------------------------------------------------------------
  def __init__(self, *args):
    unittest.TestCase.__init__(self, *args)

    parser = self.createArgParser()
    self.setupArgs(parser)
    self.parseArgs(parser.parse_args())

  #----------------------------------------------------------------------------
  def createArgParser(self):
    return argparse.ArgumentParser(description="VisGUI VTK test")

  #----------------------------------------------------------------------------
  def setupArgs(self, parser):
    parser.add_argument("-D", "--test-data-root", dest='dataDir', default=None,
      help="base directory for test data files")
    parser.add_argument("-B", "--test-baseline", dest='baseline', default=None,
      help="baseline image for the test")
    parser.add_argument("-T", "--test-temp-root", dest='tempDir', default=None,
      help="location in which to store temporary files related to testing")

  #----------------------------------------------------------------------------
  def parseArgs(self, args):
    self._dataDir = args.dataDir
    self._tempDir = args.tempDir
    self._baseline = args.baseline

  #----------------------------------------------------------------------------
  def getDataPath(self, template):
    if self._dataDir is None:
      raise Exception('data directory was not specified')
    return template.replace('$VISGUI_DATA_ROOT', self._dataDir)

  #----------------------------------------------------------------------------
  def createRenderContext(self):
    renderer = vtkRenderer()

    renderWindow = vtkRenderWindow()
    renderWindow.AddRenderer(renderer)

    interactor = self.createInteractor()
    interactor.SetRenderWindow(renderWindow)

    context = RenderContext()
    context.renderer = renderer
    context.interactor = interactor
    return context

  #----------------------------------------------------------------------------
  def createInteractor(self):
    return vtkRenderWindowInteractor()

  #----------------------------------------------------------------------------
  def compareToBaseline(self, context, threshold=50.0):
    renderWindow = context.interactor.GetRenderWindow()
    renderWindow.SetSize(300, 300)
    renderWindow.Render()

    tmp = self._tempDir if self._tempDir is not None else '.'
    outPath = os.path.join(tmp, className(self) + '-actual.png')

    windowToImageFilter = vtkWindowToImageFilter()
    windowToImageFilter.SetInput(renderWindow)

    if not os.path.isfile(outPath):
      pngWriter = vtkPNGWriter()
      pngWriter.SetFileName(outPath)
      pngWriter.SetInputConnection(windowToImageFilter.GetOutputPort())
      pngWriter.Write()
      pngWriter = None

    pngReader = vtkPNGReader()
    pngReader.SetFileName(self._baseline)

    diff = vtkImageDifference()
    diff.SetInputConnection(windowToImageFilter.GetOutputPort())
    diff.SetImageConnection(pngReader.GetOutputPort())
    diff.Update()

    self.assertLessEqual(diff.GetThresholdedError(), threshold)
