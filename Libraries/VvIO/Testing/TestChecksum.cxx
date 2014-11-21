/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <qtTest.h>

#include "../vvChecksum.h"

//-----------------------------------------------------------------------------
int testChecksum(qtTest& testObject)
{
  vvDescriptor d1;
  d1.DescriptorName = "test";
  d1.ModuleName = "test";
  d1.InstanceId = 0;
  d1.Confidence = 0.5;
  d1.Values.push_back(std::vector<float>(1, 0.3f));
  vvDescriptorRegionEntry re;
  re.TimeStamp = vgTimeStamp(1302025968.521, 3571U);
  re.ImageRegion.TopLeft.X = 300;
  re.ImageRegion.TopLeft.Y = 210;
  re.ImageRegion.BottomRight.X = 307;
  re.ImageRegion.BottomRight.Y = 214;
  d1.Region.insert(re);
  d1.TrackIds.push_back(vvTrackId(1, 1));

  vvDescriptor d2(d1);
  d2.InstanceId = 1;

  // Test uniqueness
  quint16 sum1 = vvChecksum(d1), sum2 = vvChecksum(d2);
  TEST(sum1 != sum2);

  // Test array uniqueness
  QList<vvDescriptor> qa;
  qa << d1 << d2;
  quint16 sumqa = vvChecksum(qa);
  TEST(sumqa != sum1);
  TEST(sumqa != sum2);

  // Test same checksum from either array type
  std::vector<vvDescriptor> sa;
  sa.push_back(d1);
  sa.push_back(d2);
  quint16 sumsa = vvChecksum(sa);
  TEST(sumqa == sumsa);

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  qtTest testObject;

  testObject.runSuite("Checksum Tests", testChecksum);
  return testObject.result();
}
