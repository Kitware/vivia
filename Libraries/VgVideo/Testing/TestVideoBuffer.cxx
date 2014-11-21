/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "../vgVideoBuffer.h"

#include <qtStringStream.h>
#include <qtTest.h>

#include <vgDebug.h>

QList<vgImage> images;
const int COUNT = 5, LAST = COUNT - 1;
const double FIRST_TIME = 1302909449.0;
const int IMAGE_BYTES = 720 * 468 * 3;

const char* compressionFormat;

//-----------------------------------------------------------------------------
vgVideoBuffer* buffer()
{
  static vgVideoBuffer* theBuffer = 0;
  if (!theBuffer)
    {
    theBuffer = new vgVideoBuffer(compressionFormat);
    }
  return theBuffer;
}

//-----------------------------------------------------------------------------
vgImage randomImage()
{
  QScopedArrayPointer<unsigned char> data(new unsigned char[IMAGE_BYTES]);
  for (int i = 0; i < IMAGE_BYTES; ++i)
    {
    data[i] = qrand() & 0xFF;
    }
  return vgImage(data.data(), 720, 468, 3, 3, 3 * 720, 1);
}

//-----------------------------------------------------------------------------
unsigned char subpixel(vgImage const& image, int i, int j, int plane)
{
  const unsigned char* const data = image.constData();
  const ptrdiff_t offset =
    (i * image.iStep()) + (j * image.jStep()) + (plane * image.planeStep());
  return *(data + offset);
}

//-----------------------------------------------------------------------------
void compareImages(qtTest& testObject, vgImage a, vgImage b)
{
  TEST_EQUAL(a.iCount(), b.iCount());
  TEST_EQUAL(a.jCount(), b.jCount());
  TEST_EQUAL(a.planeCount(), b.planeCount());
  if (!compressionFormat)
    {
    TEST_EQUAL(a.iStep(), b.iStep());
    TEST_EQUAL(a.jStep(), b.jStep());
    TEST_EQUAL(a.planeStep(), b.planeStep());
    }
  const unsigned char* ad, *bd;
  TEST((ad = a.constData()) != 0);
  TEST((bd = b.constData()) != 0);
  if (ad && bd)
    {
    if (compressionFormat)
      {
      // Must compare individual sub-pixels one at a time, as packing order may
      // have changed
      int p = a.planeCount();
      while (p--)
        {
        int i = a.iCount();
        while (i--)
          {
          int j = a.jCount();
          while (j--)
            {
            const int asp = subpixel(a, i, j, p);
            const int bsp = subpixel(b, i, j, p);
            if (TEST_EQUAL(asp - bsp, 0))
              {
              testObject.out() << "at " << i << ',' << j << ',' << p << '\n';
              return;
              }
            }
          }
        }
      }
    else
      {
      TEST_EQUAL(memcmp(a.constData(), b.constData(), IMAGE_BYTES), 0);
      }
    }
}

//-----------------------------------------------------------------------------
void testSeek(qtTest& testObject, double time, vg::SeekMode direction,
              vgTimeStamp expectedTime)
{
  TEST_EQUAL(buffer()->frameAt(vgTimeStamp(time), direction).time(),
             expectedTime);
}

//-----------------------------------------------------------------------------
void testIter(qtTest& testObject, int pos)
{
  vgVideoBuffer& b = *buffer();

  if (pos >= 0)
    {
    vgTimeStamp t(FIRST_TIME + double(pos), pos);
    TEST_EQUAL(b.currentTimeStamp(), t);
    vgVideoFramePtr fp = b.currentFrame();
    TEST_EQUAL(fp.time(), t);
    TEST_CALL(compareImages, fp.image(), images[pos]);
    TEST_CALL(compareImages, b.currentImage(), images[pos]);
    }
  else
    {
    vgTimeStamp t;
    TEST_EQUAL(b.currentTimeStamp(), t);
    vgVideoFramePtr fp = b.currentFrame();
    TEST_EQUAL(fp.time(), t);
    TEST_EQUAL(b.currentImage().isValid(), false);
    TEST_EQUAL(fp.image().isValid(), false);
    }
}

//-----------------------------------------------------------------------------
void testSeek(qtTest& testObject, double time, vg::SeekMode direction,
              int expectedFrameNumber)
{
  vgVideoBuffer& b = *buffer();

  vgTimeStamp expectedTimeStamp = (expectedFrameNumber >= 0 ?
     vgTimeStamp(FIRST_TIME + double(expectedFrameNumber),
                 expectedFrameNumber) :
     vgTimeStamp());

  TEST_EQUAL(b.seek(vgTimeStamp(time), direction), expectedTimeStamp);
  TEST_CALL(testIter, expectedFrameNumber);
}

//-----------------------------------------------------------------------------
int testInsertion(qtTest& testObject)
{
  vgVideoBuffer& b = *buffer();

  int i;

  // Create some images for testing
  for (i = 0; i < COUNT; ++i)
    {
    images.append(randomImage());
    }

  // Test basic insertion
  for (i = 0; i < COUNT; ++i)
    {
    vgTimeStamp t(FIRST_TIME + double(i), i);
    TEST(b.insert(t, images[i]));
    }

  // Test that multiple insertion fails
  TEST_QUIET_XFAIL(b.insert(vgTimeStamp(FIRST_TIME), images[0]));

  return 0;
}

//-----------------------------------------------------------------------------
int testMetadata(qtTest& testObject)
{
  vgVideoBuffer& b = *buffer();

  vgTimeStamp first(FIRST_TIME, 0);
  vgTimeStamp last(FIRST_TIME + double(LAST), LAST);

  TEST_EQUAL(b.frameCount(), COUNT);
  TEST_EQUAL(b.timeRange(), vgRange<vgTimeStamp>(first, last));
  TEST_EQUAL(b.firstTime(), first);
  TEST_EQUAL(b.lastTime(), last);

  return 0;
}

//-----------------------------------------------------------------------------
int testMapAccess(qtTest& testObject)
{
  vgVideoBuffer::FrameMap map = buffer()->frames();

  TEST_EQUAL(map.count(), COUNT);
  int i = 0;
  vgVideoBuffer::FrameMap::const_iterator iter = map.begin();
  while (iter != map.end())
    {
    TEST_EQUAL(iter.key(), vgTimeStamp(FIRST_TIME + double(i), i));
    TEST_EQUAL(iter.key(), iter.value().time());
    TEST_CALL(compareImages, iter.value().image(), images[i]);
    ++iter;
    ++i;
    }

  return 0;
}

//-----------------------------------------------------------------------------
int testRandomAccess(qtTest& testObject)
{
  vgVideoBuffer& b = *buffer();
  vgVideoFramePtr fp;

  for (int i = 0; i < COUNT; ++i)
    {
    vgTimeStamp ts(FIRST_TIME + double(i), i);

    // Access by time
    vgTimeStamp byTime = vgTimeStamp::fromTime(FIRST_TIME + double(i));
    fp = b.frameAt(byTime);
    TEST_EQUAL(fp.time(), ts);
    TEST_CALL(compareImages, fp.image(), images[i]);
    TEST_CALL(compareImages, b.imageAt(byTime), images[i]);

    // Access by frame number
    vgTimeStamp byFrameNumber = vgTimeStamp::fromFrameNumber(i);
    fp = b.frameAt(byFrameNumber);
    TEST_EQUAL(fp.time(), ts);
    TEST_CALL(compareImages, fp.image(), images[i]);
    TEST_CALL(compareImages, b.imageAt(byFrameNumber), images[i]);
    }

  return 0;
}

//-----------------------------------------------------------------------------
int testSeekDirect(qtTest& testObject)
{
  vgTimeStamp ta(FIRST_TIME, 0);
  vgTimeStamp tb(FIRST_TIME + 1.0, 1);
  vgTimeStamp tc(FIRST_TIME + double(LAST), LAST);
  vgTimeStamp ti;

  TEST_CALL(testSeek, FIRST_TIME + 0.4, vg::SeekNearest, ta);
  TEST_CALL(testSeek, FIRST_TIME + 0.6, vg::SeekNearest, tb);
  TEST_CALL(testSeek, FIRST_TIME + 0.6, vg::SeekNearest, tb);
  TEST_CALL(testSeek, FIRST_TIME + 0.4, vg::SeekExact, ti);

  TEST_CALL(testSeek, FIRST_TIME + 1.0, vg::SeekLowerBound, tb);
  TEST_CALL(testSeek, FIRST_TIME + 1.0, vg::SeekUpperBound, tb);

  TEST_CALL(testSeek, FIRST_TIME + 0.4, vg::SeekLowerBound, tb);
  TEST_CALL(testSeek, FIRST_TIME + 0.6, vg::SeekUpperBound, ta);

  TEST_CALL(testSeek, FIRST_TIME - 1.0, vg::SeekNearest, ta);
  TEST_CALL(testSeek, FIRST_TIME - 1.0, vg::SeekLowerBound, ta);
  TEST_CALL(testSeek, FIRST_TIME - 1.0, vg::SeekUpperBound, ti);

  TEST_CALL(testSeek, FIRST_TIME * 99.0, vg::SeekNearest, tc);
  TEST_CALL(testSeek, FIRST_TIME * 99.0, vg::SeekLowerBound, ti);
  TEST_CALL(testSeek, FIRST_TIME * 99.0, vg::SeekUpperBound, tc);

  return 0;
}

//-----------------------------------------------------------------------------
int testSequentialAccess(qtTest& testObject)
{
  vgVideoBuffer& b = *buffer();
  int i;

  // Test forward access (and advance past end)
  b.rewind();
  for (i = 0; i < COUNT; ++i)
    {
    TEST_CALL(testIter, i);
    TEST_EQUAL(b.advance().IsValid(), (i + 1) != COUNT);
    }
  TEST_CALL(testIter, -1);

  // Test reverse access
  while (--i)
    {
    TEST_EQUAL(b.recede().IsValid(), true);
    TEST_CALL(testIter, i);
    }

  // Test rewind
  while (b.advance().IsValid())
    {
    // Empty loop; just calling advance() until at end of clip
    }
  b.rewind();
  TEST_CALL(testIter, 0);

  return 0;
}

//-----------------------------------------------------------------------------
int testSeekIter(qtTest& testObject)
{
  TEST_CALL(testSeek, FIRST_TIME + 0.4, vg::SeekNearest, 0);
  TEST_CALL(testSeek, FIRST_TIME + 0.6, vg::SeekNearest, 1);
  TEST_CALL(testSeek, FIRST_TIME + 0.6, vg::SeekNearest, 1);
  TEST_CALL(testSeek, FIRST_TIME + 0.4, vg::SeekExact, -1);

  TEST_CALL(testSeek, FIRST_TIME + 1.0, vg::SeekLowerBound, 1);
  TEST_CALL(testSeek, FIRST_TIME + 1.0, vg::SeekUpperBound, 1);

  TEST_CALL(testSeek, FIRST_TIME + 0.4, vg::SeekLowerBound, 1);
  TEST_CALL(testSeek, FIRST_TIME + 0.6, vg::SeekUpperBound, 0);

  TEST_CALL(testSeek, FIRST_TIME - 1.0, vg::SeekNearest, 0);
  TEST_CALL(testSeek, FIRST_TIME - 1.0, vg::SeekLowerBound, 0);
  TEST_CALL(testSeek, FIRST_TIME - 1.0, vg::SeekUpperBound, -1);

  TEST_CALL(testSeek, FIRST_TIME * 99.0, vg::SeekNearest, LAST);
  TEST_CALL(testSeek, FIRST_TIME * 99.0, vg::SeekLowerBound, -1);
  TEST_CALL(testSeek, FIRST_TIME * 99.0, vg::SeekUpperBound, LAST);

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char** argv)
{
  qtTest testObject;

  compressionFormat = (argc > 1 ? argv[1] : 0);

  // Create the buffer instance and stuff it in a scoped pointer so it will
  // be automatically cleaned up... a bit of a hack, but works
  QScopedPointer<vgVideoBuffer> theBuffer(buffer());

  // Run insertion tests; if this fails, give up, as the remaining tests will
  // all go quite badly...
  if (testObject.runSuite("Insertion Tests", testInsertion))
    {
    return testObject.result();
    }

  // Insertion worked and we (theoretically) have a good buffer, so now we can
  // run the remaining tests
  testObject.runSuite("Metadata Tests", testMetadata);
  testObject.runSuite("Map Access Tests", testMapAccess);
  testObject.runSuite("Random Access Tests", testRandomAccess);
  testObject.runSuite("Direct Seek Tests", testSeekDirect);
  testObject.runSuite("Sequential Access Tests", testSequentialAccess);
  testObject.runSuite("Iterator Seek Tests", testSeekIter);

  // Done
  return testObject.result();
}
