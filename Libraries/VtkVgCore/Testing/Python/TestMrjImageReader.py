from vtkRenderingCorePython import *
import vtkRenderingOpenGLPython

import vtkVgCorePython
from visgui.testing import VtkTest

class TestMrjImageReader(VtkTest):
  def testRead(self):
    mrjReader = vtkVgCorePython.vtkVgMultiResJpgImageReader2()
    mrjReader.SetFileName(self.getDataPath('$VISGUI_DATA_ROOT/sample.mrj'))
    mrjReader.SetLevel(5)
    mrjReader.Update()

    imageActor = vtkImageActor()
    imageActor.SetInputData(mrjReader.GetOutput())

    context = self.createRenderContext()
    context.renderer.AddActor(imageActor)

    self.compareToBaseline(context)

if __name__ == "__main__":
    VtkTest.main()
