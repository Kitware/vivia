import unittest

from vtkVgCorePython import vtkVgTimeStamp

class TestTimeStamp(unittest.TestCase):
  def testConstruction(self):
    # Constructing with time only
    t1 = vtkVgTimeStamp(123456.0)
    self.assertTrue(t1.IsValid())
    self.assertTrue(t1.HasTime())
    self.assertFalse(t1.HasFrameNumber())
    self.assertEqual(t1.GetTime(), 123456.0)
    # Constructing with time and frame number
    t2 = vtkVgTimeStamp(123456.0, 42)
    self.assertTrue(t2.IsValid())
    self.assertTrue(t2.HasTime())
    self.assertTrue(t2.HasFrameNumber())
    self.assertEqual(t2.GetTime(), 123456.0)
    self.assertEqual(t2.GetFrameNumber(), 42)
    # Constructing minimum time
    tmin = vtkVgTimeStamp(False)
    tmax = vtkVgTimeStamp(True)
    self.assertTrue(tmin.IsMinTime())
    self.assertTrue(tmax.IsMaxTime())

  def testTime(self):
    t = vtkVgTimeStamp()
    # Test initial state
    self.assertFalse(t.IsValid())
    self.assertFalse(t.HasTime())
    # Test setting and getting time
    t.SetTime(123456.0)
    self.assertTrue(t.IsValid())
    self.assertTrue(t.HasTime())
    self.assertEqual(t.GetTime(), 123456.0)

  def testFrameNumber(self):
    t = vtkVgTimeStamp()
    # Test initial state
    self.assertFalse(t.IsValid())
    self.assertFalse(t.HasFrameNumber())
    # Test setting and getting frame number
    t.SetFrameNumber(42)
    self.assertTrue(t.IsValid())
    self.assertTrue(t.HasFrameNumber())
    self.assertEqual(t.GetFrameNumber(), 42)

  def testMinMax(self):
    t = vtkVgTimeStamp()
    self.assertFalse(t.IsValid())
    t.SetToMinTime()
    self.assertTrue(t.IsMinTime())
    t.SetToMaxTime()
    self.assertTrue(t.IsMaxTime())

  def testReset(self):
    t = vtkVgTimeStamp(0.0, 0);
    self.assertTrue(t.IsValid())
    self.assertTrue(t.HasTime())
    self.assertTrue(t.HasFrameNumber())
    t.Reset()
    self.assertFalse(t.IsValid())
    self.assertFalse(t.HasTime())
    self.assertFalse(t.HasFrameNumber())

if __name__ == "__main__":
    unittest.main()
