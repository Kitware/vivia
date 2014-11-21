/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QUrl>

#include <qtTest.h>

#include "../vvReader.h"
#include "../vvXmlWriter.h"

#include "TestReadWrite.h"

QString testFileBase;

//-----------------------------------------------------------------------------
int testTrack(qtTest& testObject)
{
  QFile fileA(testFileBase + "-a.vst.xml");
  if (!fileA.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file A\n";
    return 1;
    }
  QString inDataA = QString::fromUtf8(fileA.readAll());

  QFile fileB(testFileBase + "-b.vst.xml");
  if (!fileB.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file B\n";
    return 1;
    }
  QString inDataB = QString::fromUtf8(fileB.readAll());

  // Read first header
  vvReader reader;
  vvHeader header;
  TEST(reader.setInput(inDataA));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::Tracks);

  // Read tracks
  QList<vvTrack> tracks;
  TEST(reader.readTracks(tracks));
  if (TEST_EQUAL(tracks.count(), 2) == 0)
    {
    // Verify tracks
    TEST_CALL(testTrack1, tracks[0]);
    TEST_CALL(testTrack2, tracks[1]);
    }

  // Write tracks
  QString outDataA;
  QTextStream outA(&outDataA);
  vvWriter(outA, vvWriter::Xml) << tracks;

  // Verify output
  TEST_EQUAL(outDataA, inDataA);

  // Read second header
  TEST(reader.setInput(inDataB));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::Tracks);

  // Read track
  TEST(reader.readTracks(tracks));
  if (TEST_EQUAL(tracks.count(), 1) == 0)
    {
    // Verify track
    TEST_CALL(testTrack3, tracks[0]);
    }

  // Write track
  QString outDataB;
  QTextStream outB(&outDataB);
  vvWriter(outB, vvWriter::Xml) << tracks;

  // Verify output
  TEST_EQUAL(outDataB, inDataB);

  return 0;
}

//-----------------------------------------------------------------------------
int testDescriptor(qtTest& testObject)
{
  QFile file(testFileBase + ".vsd.xml");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file\n";
    return 1;
    }
  QString inData = QString::fromUtf8(file.readAll());

  // Read header
  vvReader reader;
  vvHeader header;
  TEST(reader.setInput(inData));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::Descriptors);

  // Read descriptors
  QList<vvDescriptor> descriptors;
  TEST(reader.readDescriptors(descriptors));
  if (TEST_EQUAL(descriptors.count(), 3) == 0)
    {
    // Verify descriptors
    TEST_CALL(testDescriptor1, descriptors[0]);
    TEST_CALL(testDescriptor2, descriptors[1]);
    TEST_CALL(testDescriptor3, descriptors[2]);
    }

  // Write descriptors
  QString outData;
  QTextStream out(&outData);
  vvWriter(out, vvWriter::Xml) << descriptors;

  // Verify output
  TEST_EQUAL(outData, inData);

  return 0;
}

//-----------------------------------------------------------------------------
int testQueryPlan(qtTest& testObject)
{
  QFile fileA(testFileBase + "-a.vqp.xml");
  if (!fileA.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file A\n";
    return 1;
    }
  QString inDataA = QString::fromUtf8(fileA.readAll());

  QFile fileB(testFileBase + "-b.vqp.xml");
  if (!fileB.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file B(in)\n";
    return 1;
    }
  QString inDataB = QString::fromUtf8(fileB.readAll());

  QFile fileC(testFileBase + "-c.vqp.xml");
  if (!fileC.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file C\n";
    return 1;
    }
  QString inDataC = QString::fromUtf8(fileC.readAll());

  vvReader reader;
  vvHeader header;
  vvQueryInstance query;

  // Read first header
  TEST(reader.setInput(inDataA));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryPlan);

  // Read and verify first plan
  TEST(reader.readQueryPlan(query));
  TEST_CALL(testQueryPlan1, query);

  // Write first plan
  QString outDataA;
  QTextStream outA(&outDataA);
  vvWriter(outA, vvWriter::Xml) << query;

  // Verify output
  TEST_EQUAL(outDataA, inDataA);

  // Read second header
  TEST(reader.setInput(inDataB));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryPlan);

  // Read and verify second plan
  TEST(reader.readQueryPlan(query));
  TEST_CALL(testQueryPlan2, query);

  // Write second plan
  QString outDataB;
  QTextStream outB(&outDataB);
  vvWriter(outB, vvWriter::Xml) << query;

  // Verify output
  TEST_EQUAL(outDataB, inDataB);

  // Read third header
  TEST(reader.setInput(inDataC));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryPlan);

  // Read and verify third plan
  TEST(reader.readQueryPlan(query));
  TEST_CALL(testQueryPlan3, query);

  // Write third plan
  QString outDataC;
  QTextStream outC(&outDataC);
  vvWriter(outC, vvWriter::Xml) << query;

  // Verify output
  TEST_EQUAL(outDataC, inDataC);

  return 0;
}

//-----------------------------------------------------------------------------
int testQueryResult(qtTest& testObject)
{
  QFile file(testFileBase + ".vqr.xml");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file\n";
    return 1;
    }
  QString inData = QString::fromUtf8(file.readAll());

  // Read header
  vvReader reader;
  vvHeader header;
  TEST(reader.setInput(inData));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryResults);

  // Read results
  QList<vvQueryResult> results;
  TEST(reader.readQueryResults(results));
  if (TEST_EQUAL(results.count(), 2) == 0)
    {
    // Verify results
    TEST_CALL(testQueryResult1, results[0], static_cast<uint>(-1));
    TEST_CALL(testQueryResult2, results[1], static_cast<uint>(-1));
    }

  // Write results
  QString outData;
  QTextStream out(&outData);
  vvWriter(out, vvWriter::Xml) << results;

  // Verify output
  TEST_EQUAL(outData, inData);

  return 0;
}

//-----------------------------------------------------------------------------
int testGeoPoly(qtTest& testObject)
{
  QFile fileA(testFileBase + "-a.vgp.xml");
  if (!fileA.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file A\n";
    return 1;
    }
  QString inDataA = QString::fromUtf8(fileA.readAll());

  QFile fileB(testFileBase + "-b.vgp.xml");
  if (!fileB.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file B\n";
    return 1;
    }
  QString inDataB = QString::fromUtf8(fileB.readAll());

  // Read first polygon
  vvReader reader;
  vgGeocodedPoly geoPoly;
  TEST(reader.setInput(inDataA));
  TEST(reader.readGeoPoly(geoPoly));

  // Verify polygon
  TEST_CALL(testGeoPoly1, geoPoly);

  // Write first polygon
  QString outDataA;
  QTextStream outA(&outDataA);
  vvWriter(outA, vvWriter::Xml) << geoPoly;

  // Verify output
  TEST_EQUAL(outDataA, inDataA);

  // Read second polygon
  TEST(reader.setInput(inDataB));
  TEST(reader.readGeoPoly(geoPoly));

  // Verify polygon
  TEST_CALL(testGeoPoly2, geoPoly);

  // Write second polygon
  QString outDataB;
  QTextStream outB(&outDataB);
  vvWriter(outB, vvWriter::Xml) << geoPoly;

  // Verify output
  TEST_EQUAL(outDataB, inDataB);

  return 0;
}

//-----------------------------------------------------------------------------
int testEventSetInfo(qtTest& testObject)
{
  QFile file(testFileBase + ".vem.xml");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file\n";
    return 1;
    }
  QString inData = QString::fromUtf8(file.readAll());

  // Read header
  vvReader reader;
  vvHeader header;
  TEST(reader.setInput(inData));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::EventSetInfo);

  // Read event set info
  vvEventSetInfo info;
  TEST(reader.readEventSetInfo(info));

  // Verify event set info
  TEST_CALL(testEventSetInfo, info);

  // Write event set info
  QString outData;
  QTextStream out(&outData);
  vvWriter(out, vvWriter::Xml) << info;

  // Verify output
  TEST_EQUAL(outData, inData);

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  qtTest testObject;

  if (argc < 2)
    {
    testObject.out() << "invocation error, path to test data files required\n";
    return 1;
    }
  testFileBase = QString::fromLocal8Bit(argv[1]);

  testObject.runSuite("Track Tests",                testTrack);
  testObject.runSuite("Descriptor Tests",           testDescriptor);
  testObject.runSuite("Query Plan Tests",           testQueryPlan);
  testObject.runSuite("Query Result Tests",         testQueryResult);
  testObject.runSuite("Geospatial Polygon Tests",   testGeoPoly);
  testObject.runSuite("Event Set Info Tests",       testEventSetInfo);
  return testObject.result();
}
