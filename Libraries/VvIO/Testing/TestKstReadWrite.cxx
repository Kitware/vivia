// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <QByteArray>
#include <QFile>

#include <qtKstReader.h>
#include <qtTest.h>

#include "../vvKstReader.h"
#include "../vvKstWriter.h"

#include "TestReadWrite.h"

QString testFileBase;

//-----------------------------------------------------------------------------
int testTrack(qtTest& testObject)
{
  QFile fileA(testFileBase + "-a.vst");
  if (!fileA.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file A\n";
    return 1;
    }
  QString inDataA = QString::fromUtf8(fileA.readAll());

  QFile fileB(testFileBase + "-b.vst");
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
  TEST_EQUAL(header.version, vvKstWriter::TracksVersion);

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
  vvWriter writerA(outA, vvWriter::Kst);
  writerA << vvHeader::Tracks << tracks;

  // Verify output
  TEST_EQUAL(outDataA, inDataA);

  // Test terse output
  QString terseDataA;
  QTextStream terseA(&terseDataA);
  vvKstWriter terseWriterA(terseA, false);
  terseWriterA << tracks;

  qtKstReader terseKstA(terseDataA);
  vvKstReader terseReader;
  TEST(terseKstA.isValid());
  TEST(terseReader.readTracks(terseKstA, tracks,
                              vvKstWriter::TracksVersion));

  if (TEST_EQUAL(tracks.count(), 2) == 0)
    {
    // Verify tracks
    TEST_CALL(testTrack1, tracks[0]);
    TEST_CALL(testTrack2, tracks[1]);
    }

  // Read second header
  TEST(reader.setInput(inDataB));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::Tracks);
  TEST_EQUAL(header.version, vvKstWriter::TracksVersion);

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
  vvWriter writerB(outB, vvWriter::Kst);
  writerB << vvHeader::Tracks << tracks;

  // Verify output
  TEST_EQUAL(outDataB, inDataB);

  // Test terse output
  QString terseDataB;
  QTextStream terseB(&terseDataB);
  vvKstWriter terseWriterB(terseB, false);
  terseWriterB << tracks;

  qtKstReader terseKstB(terseDataB);
  TEST(terseKstB.isValid());
  TEST(terseReader.readTracks(terseKstB, tracks,
                              vvKstWriter::TracksVersion));

  if (TEST_EQUAL(tracks.count(), 1) == 0)
    {
    // Verify track
    TEST_CALL(testTrack3, tracks[0]);
    }

  return 0;
}

//-----------------------------------------------------------------------------
int testDescriptor(qtTest& testObject)
{
  QFile file(testFileBase + ".vsd");
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
  TEST_EQUAL(header.version, vvKstWriter::DescriptorsVersion);

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
  vvWriter writer(out, vvWriter::Kst);
  writer << vvHeader::Descriptors << descriptors;

  // Verify output
  TEST_EQUAL(outData, inData);

  // Test terse output
  QString terseData;
  QTextStream terse(&terseData);
  vvKstWriter terseWriter(terse, false);
  terseWriter << descriptors;

  qtKstReader terseKst(terseData);
  vvKstReader terseReader;
  TEST(terseKst.isValid());
  TEST(terseReader.readDescriptors(terseKst, descriptors,
                                   vvKstWriter::DescriptorsVersion));

  if (TEST_EQUAL(descriptors.count(), 3) == 0)
    {
    // Verify descriptors
    TEST_CALL(testDescriptor1, descriptors[0]);
    TEST_CALL(testDescriptor2, descriptors[1]);
    TEST_CALL(testDescriptor3, descriptors[2]);
    }

  return 0;
}

//-----------------------------------------------------------------------------
int testQueryPlan(qtTest& testObject)
{
  QFile fileA(testFileBase + "-a.vqp");
  if (!fileA.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file A\n";
    return 1;
    }
  QString inDataA = QString::fromUtf8(fileA.readAll());

  QFile fileB(testFileBase + "-b.vqp");
  if (!fileB.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file B(in)\n";
    return 1;
    }
  QString inDataB = QString::fromUtf8(fileB.readAll());

  QFile fileBO(testFileBase + "-bo.vqp");
  if (!fileBO.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file B(out)\n";
    return 1;
    }
  QString refDataB = QString::fromUtf8(fileBO.readAll());

  QFile fileC(testFileBase + "-c.vqp");
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
  TEST_EQUAL(header.version, vvKstWriter::QueryPlanVersion);

  // Read and verify first plan
  TEST(reader.readQueryPlan(query));
  TEST_CALL(testQueryPlan1, query);

  // Write first plan
  QString outDataA;
  QTextStream outA(&outDataA);
  vvWriter writerA(outA, vvWriter::Kst);
  writerA << vvHeader::QueryPlan << query;

  // Verify output
  TEST_EQUAL(outDataA, inDataA);

  // Test terse output (first plan)
  QString terseDataA;
  QTextStream terseA(&terseDataA);
  vvKstWriter terseWriterA(terseA, false);
  terseWriterA << query;

  qtKstReader terseKstA(terseDataA);
  vvKstReader terseReader;
  TEST(terseKstA.isValid());
  TEST(terseReader.readQueryPlan(terseKstA, query,
                                 vvKstWriter::QueryPlanVersion));
  TEST_CALL(testQueryPlan1, query);

  // Read second header
  TEST(reader.setInput(inDataB));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryPlan);
  TEST_EQUAL(header.version, 0u); // test file B is old version

  // Read and verify second plan
  TEST(reader.readQueryPlan(query));
  TEST_CALL(testQueryPlan2, query);

  // Write second plan
  QString outDataB;
  QTextStream outB(&outDataB);
  vvWriter writerB(outB, vvWriter::Kst);
  writerB << vvHeader::QueryPlan << query;

  // Verify output
  TEST_EQUAL(outDataB, refDataB);

  // Test terse output (second plan)
  QString terseDataB;
  QTextStream terseB(&terseDataB);
  vvKstWriter terseWriterB(terseB, false);
  terseWriterB << query;

  qtKstReader terseKstB(terseDataB);
  TEST(terseKstB.isValid());
  TEST(terseReader.readQueryPlan(terseKstB, query,
                                 vvKstWriter::QueryPlanVersion));
  TEST_CALL(testQueryPlan2, query);

  // Read third header
  TEST(reader.setInput(inDataC));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryPlan);
  TEST_EQUAL(header.version, vvKstWriter::QueryPlanVersion);

  // Read and verify third plan
  TEST(reader.readQueryPlan(query));
  TEST_CALL(testQueryPlan3, query);

  // Write third plan
  QString outDataC;
  QTextStream outC(&outDataC);
  vvWriter writerC(outC, vvWriter::Kst);
  writerC << vvHeader::QueryPlan << query;

  // Verify output
  TEST_EQUAL(outDataC, inDataC);

  // Test terse output (third plan)
  QString terseDataC;
  QTextStream terseC(&terseDataC);
  vvKstWriter terseWriterC(terseC, false);
  terseWriterC << query;

  qtKstReader terseKstC(terseDataC);
  TEST(terseKstC.isValid());
  TEST(terseReader.readQueryPlan(terseKstC, query,
                                 vvKstWriter::QueryPlanVersion));
  TEST_CALL(testQueryPlan3, query);

  return 0;
}

//-----------------------------------------------------------------------------
int testQueryResult(qtTest& testObject)
{
  QFile file1(testFileBase + "-1.vqr");
  if (!file1.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open v1 test file\n";
    return 1;
    }
  QString inData1 = QString::fromUtf8(file1.readAll());

  QFile file2(testFileBase + "-2.vqr");
  if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open v2 test file\n";
    return 1;
    }
  QString inData2 = QString::fromUtf8(file2.readAll());

  // Read v1 header
  vvReader reader;
  vvHeader header;
  TEST(reader.setInput(inData1));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryResults);
  TEST_EQUAL(header.version, 1u);

  // Read v1 results
  QList<vvQueryResult> results;
  TEST(reader.readQueryResults(results));
  if (TEST_EQUAL(results.count(), 2) == 0)
    {
    // Verify v1 results
    TEST_EQUAL(results[0].Rank, -1LL);
    TEST_EQUAL(results[1].Rank, -1LL);
    results[0].Rank = 0;
    results[1].Rank = 1;
    TEST_CALL(testQueryResult1, results[0], header.version);
    TEST_CALL(testQueryResult2, results[1], header.version);
    }

  // Read v2 header
  TEST(reader.setInput(inData2));
  TEST(reader.readHeader(header));
  TEST_EQUAL(header.type, vvHeader::QueryResults);
  TEST_EQUAL(header.version, vvKstWriter::QueryResultsVersion);

  // Read v2 results
  TEST(reader.readQueryResults(results));
  if (TEST_EQUAL(results.count(), 2) == 0)
    {
    // Verify v1 results
    TEST_CALL(testQueryResult1, results[0], header.version);
    TEST_CALL(testQueryResult2, results[1], header.version);
    }

  // Write results
  QString outData;
  QTextStream out(&outData);
  vvWriter writer(out, vvWriter::Kst);
  writer << vvHeader::QueryResults << results;

  // Verify output
  TEST_EQUAL(outData, inData2);

  // Test terse output
  QString terseData;
  QTextStream terse(&terseData);
  vvKstWriter terseWriter(terse, false);
  terseWriter << results;

  qtKstReader terseKst(terseData);
  vvKstReader terseReader;
  TEST(terseKst.isValid());
  TEST(terseReader.readQueryResults(terseKst, results,
                                    vvKstWriter::QueryResultsVersion));

  if (TEST_EQUAL(results.count(), 2) == 0)
    {
    // Verify results
    TEST_CALL(testQueryResult1, results[0], vvKstWriter::QueryResultsVersion);
    TEST_CALL(testQueryResult2, results[1], vvKstWriter::QueryResultsVersion);
    }

  return 0;
}

//-----------------------------------------------------------------------------
int testGeoPoly(qtTest& testObject)
{
  QFile fileA(testFileBase + "-a.vgp");
  if (!fileA.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file A\n";
    return 1;
    }

  QString inDataA = QString::fromUtf8(fileA.readAll());
  qtKstReader kstA(inDataA);
  if (!kstA.isValid())
    {
    testObject.out() << "error parsing test file A (KST invalid)\n";
    return 1;
    }

  QFile fileB(testFileBase + "-b.vgp");
  if (!fileB.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    testObject.out() << "unable to open test file B\n";
    return 1;
    }

  QString inDataB = QString::fromUtf8(fileB.readAll());
  qtKstReader kstB(inDataB);
  if (!kstB.isValid())
    {
    testObject.out() << "error parsing test file B (KST invalid)\n";
    return 1;
    }

  // Read first header
  int version;
  if (!(kstA.readInt(version) && kstA.nextRecord()))
    {
    testObject.out() << "error parsing test file (unable to read version)\n";
    return 1;
    }

  // Read first polygon
  vvKstReader reader;
  vgGeocodedPoly geoPoly;
  TEST(reader.readGeoPoly(kstA, geoPoly, version));

  // Verify polygon
  TEST_CALL(testGeoPoly1, geoPoly);

  // Write first polygon
  QString outDataA = QString::number(vvKstWriter::GeoPolyVersion) + ";\n";
  QTextStream outA(&outDataA);
  vvKstWriter writerA(outA);
  writerA << geoPoly;

  // Verify output
  TEST_EQUAL(outDataA + ";\n", inDataA);

  // Test terse output (first polygon)
  QString terseDataA;
  QTextStream terseA(&terseDataA);
  vvKstWriter terseWriterA(terseA, false);
  terseWriterA << geoPoly;

  qtKstReader terseKstA(terseDataA + ';');
  TEST(terseKstA.isValid());
  TEST(reader.readGeoPoly(terseKstA, geoPoly, vvKstWriter::GeoPolyVersion));

  // Verify polygon
  TEST_CALL(testGeoPoly1, geoPoly);

  // Read second header
  if (!(kstB.readInt(version) && kstB.nextRecord()))
    {
    testObject.out() << "error parsing test file (unable to read version)\n";
    return 1;
    }

  // Read second polygon
  TEST(reader.readGeoPoly(kstB, geoPoly, version));

  // Verify polygon
  TEST_CALL(testGeoPoly2, geoPoly);

  // Write second polygon
  QString outDataB = QString::number(vvKstWriter::GeoPolyVersion) + ";\n";
  QTextStream outB(&outDataB);
  vvKstWriter writerB(outB);
  writerB << geoPoly;

  // Verify output
  TEST_EQUAL(outDataB + ";\n", inDataB);

  // Test terse output (second polygon)
  QString terseDataB;
  QTextStream terseB(&terseDataB);
  vvKstWriter terseWriterB(terseB, false);
  terseWriterB << geoPoly;

  qtKstReader terseKstB(terseDataB + ';');
  TEST(terseKstB.isValid());
  TEST(reader.readGeoPoly(terseKstB, geoPoly, vvKstWriter::GeoPolyVersion));

  // Verify polygon
  TEST_CALL(testGeoPoly2, geoPoly);

  return 0;
}

//-----------------------------------------------------------------------------
int testEventSetInfo(qtTest& testObject)
{
  QFile file(testFileBase + ".vem");
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
  TEST_EQUAL(header.version, vvKstWriter::EventSetInfoVersion);

  // Read event set info
  vvEventSetInfo info;
  TEST(reader.readEventSetInfo(info));

  // Verify event set info
  TEST_CALL(testEventSetInfo, info);

  // Write event set info
  QString outData;
  QTextStream out(&outData);
  vvKstWriter writer(out);
  writer << vvHeader::EventSetInfo << info;

  // Verify output
  TEST_EQUAL(outData, inData);

  // Test terse output
  QString terseData;
  QTextStream terse(&terseData);
  vvKstWriter terseWriter(terse, false);
  terseWriter << info;

  qtKstReader terseKst(terseData);
  vvKstReader terseReader;
  TEST(terseKst.isValid());
  TEST(terseReader.readEventSetInfo(terseKst, info,
                                    vvKstWriter::EventSetInfoVersion));

  // Verify event set info
  TEST_CALL(testEventSetInfo, info);

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
