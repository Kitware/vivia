// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <qtTest.h>

#include "../vgTimeStamp.h"

static const double TestTimeValue = 1302025968.521;
static const unsigned int TestFrameNumber = 3571;

//-----------------------------------------------------------------------------
void testTimestamp(qtTest& testObject, const vgTimeStamp& ts,
                   bool xht, bool xhf)
{
  TEST_EQUAL(ts.IsValid(), xht || xhf);
  TEST_EQUAL(ts.HasTime(), xht);
  TEST_EQUAL(ts.HasFrameNumber(), xhf);
  if (xht)
    TEST_EQUAL(ts.Time, TestTimeValue);
  else
    TEST_EQUAL(ts.Time, vgTimeStamp::InvalidTime());
  if (xhf)
    TEST_EQUAL(ts.FrameNumber, TestFrameNumber);
  else
    TEST_EQUAL(ts.FrameNumber, vgTimeStamp::InvalidFrameNumber());
}

//-----------------------------------------------------------------------------
int testCtor(qtTest& testObject)
{
  vgTimeStamp ts1(TestTimeValue);
  TEST_CALL(testTimestamp, ts1, true, false);

  vgTimeStamp ts2(TestFrameNumber);
  TEST_CALL(testTimestamp, ts2, false, true);

  vgTimeStamp ts3(TestTimeValue, TestFrameNumber);
  TEST_CALL(testTimestamp, ts3, true, true);

  vgTimeStamp ts4;
  TEST_CALL(testTimestamp, ts4, false, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testModification(qtTest& testObject)
{
  vgTimeStamp ts;

  ts.Time = TestTimeValue;
  TEST_CALL(testTimestamp, ts, true, false);

  ts.FrameNumber = TestFrameNumber;
  TEST_CALL(testTimestamp, ts, true, true);

  ts.Time = vgTimeStamp::InvalidTime();
  TEST_CALL(testTimestamp, ts, false, true);

  ts.FrameNumber = vgTimeStamp::InvalidFrameNumber();
  TEST_CALL(testTimestamp, ts, false, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testComparison(qtTest& testObject)
{
  // Comparing time only
  vgTimeStamp ts1(TestTimeValue), ts2(TestTimeValue + 15.0);
  TEST_EQUAL(ts1 < ts2, true);
  TEST_EQUAL(ts1 < ts1, false);
  TEST_EQUAL(ts2 < ts1, false);

  // Comparing frame number only
  vgTimeStamp ts3(TestFrameNumber), ts4(TestFrameNumber + 4U);
  TEST_EQUAL(ts3 < ts4, true);
  TEST_EQUAL(ts3 < ts3, false);
  TEST_EQUAL(ts4 < ts3, false);

  // Comparing with both
  vgTimeStamp ts5(TestTimeValue, TestFrameNumber);
  vgTimeStamp ts6(TestTimeValue + 15.0, TestFrameNumber + 4U);
  TEST_EQUAL(ts5 < ts6, true);
  TEST_EQUAL(ts5 < ts5, false);
  TEST_EQUAL(ts6 < ts5, false);

  // Comparing complete with partial
  TEST_EQUAL(ts5 < ts2, true);
  TEST_EQUAL(ts5 < ts4, true);
  TEST_EQUAL(ts5 < ts1, false);
  TEST_EQUAL(ts5 < ts3, false);
  TEST_EQUAL(ts6 < ts2, false);
  TEST_EQUAL(ts6 < ts4, false);
  TEST_EQUAL(ts6 < ts1, false);
  TEST_EQUAL(ts6 < ts3, false);

  // Unsupported comparisons
  TEST_EQUAL(ts1 < ts3, false);
  TEST_EQUAL(ts1 < ts4, false);
  TEST_EQUAL(ts2 < ts3, false);
  TEST_EQUAL(ts2 < ts4, false);
  TEST_EQUAL(ts3 < ts1, false);
  TEST_EQUAL(ts3 < ts2, false);
  TEST_EQUAL(ts4 < ts1, false);
  TEST_EQUAL(ts4 < ts2, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testEquality(qtTest& testObject)
{
  // Test inequality due to valid/invalid member mismatch
  vgTimeStamp ts0;
  vgTimeStamp ts1(TestTimeValue);
  vgTimeStamp ts2(TestFrameNumber);
  vgTimeStamp ts3(TestTimeValue, TestFrameNumber);
  TEST(ts1 != ts0);
  TEST(ts2 != ts0);
  TEST(ts3 != ts0);
  TEST(ts1 != ts2);
  TEST(ts1 != ts3);
  TEST(ts2 != ts3);

  // Test equality
  vgTimeStamp ts4(TestTimeValue);
  vgTimeStamp ts5(TestFrameNumber);
  vgTimeStamp ts6(TestTimeValue, TestFrameNumber);
  TEST(ts1 == ts4);
  TEST(ts2 == ts5);
  TEST(ts3 == ts6);

  // Test inequality due to value differences
  vgTimeStamp ts7(TestTimeValue + 15.0);
  vgTimeStamp ts8(TestFrameNumber + 4U);
  vgTimeStamp ts9(TestTimeValue + 15.0, TestFrameNumber + 4U);
  vgTimeStamp ts10(TestTimeValue + 15.0, TestFrameNumber);
  vgTimeStamp ts11(TestTimeValue, TestFrameNumber + 4U);
  TEST(ts7 != ts1);
  TEST(ts7 != ts2);
  TEST(ts7 != ts3);
  TEST(ts8 != ts1);
  TEST(ts8 != ts2);
  TEST(ts8 != ts3);
  TEST(ts9 != ts1);
  TEST(ts9 != ts2);
  TEST(ts9 != ts3);
  TEST(ts10 != ts1);
  TEST(ts10 != ts2);
  TEST(ts10 != ts3);
  TEST(ts11 != ts1);
  TEST(ts11 != ts2);
  TEST(ts11 != ts3);

  return 0;
}

//-----------------------------------------------------------------------------
int testFuzzyEquality(qtTest& testObject)
{
  vgTimeStamp ts0;
  vgTimeStamp ts1(TestTimeValue);
  vgTimeStamp ts2(TestFrameNumber);
  vgTimeStamp ts3(TestTimeValue, TestFrameNumber);

  // Test self-equality
  TEST(ts0.FuzzyEquals(ts0) == true);
  TEST(ts1.FuzzyEquals(ts1) == true);
  TEST(ts2.FuzzyEquals(ts2) == true);
  TEST(ts3.FuzzyEquals(ts3) == true);

  // Test comparison with valid/invalid member mismatch
  TEST(ts0.FuzzyEquals(ts1) == false);
  TEST(ts0.FuzzyEquals(ts2) == false);
  TEST(ts0.FuzzyEquals(ts3) == false);
  TEST(ts1.FuzzyEquals(ts2) == false);
  TEST(ts1.FuzzyEquals(ts3) == true);
  TEST(ts2.FuzzyEquals(ts3) == true);

  // Test equality within tolerance
  vgTimeStamp ts4a(TestTimeValue + 5e-6);
  vgTimeStamp ts4b(TestTimeValue - 5e-6);
  vgTimeStamp ts5a(TestTimeValue + 5e-6, TestFrameNumber);
  vgTimeStamp ts5b(TestTimeValue - 5e-6, TestFrameNumber);
  TEST(ts4a.FuzzyEquals(ts1) == true);
  TEST(ts4a.FuzzyEquals(ts3) == true);
  TEST(ts4b.FuzzyEquals(ts1) == true);
  TEST(ts4b.FuzzyEquals(ts3) == true);
  TEST(ts5a.FuzzyEquals(ts1) == true);
  TEST(ts5a.FuzzyEquals(ts3) == true);
  TEST(ts5b.FuzzyEquals(ts1) == true);
  TEST(ts5b.FuzzyEquals(ts3) == true);

  // Test inequality
  vgTimeStamp ts6a(TestTimeValue + 15e-6);
  vgTimeStamp ts6b(TestTimeValue - 15e-6);
  vgTimeStamp ts7a(TestTimeValue + 15e-6, TestFrameNumber);
  vgTimeStamp ts7b(TestTimeValue - 15e-6, TestFrameNumber);
  TEST(ts6a.FuzzyEquals(ts1) == false);
  TEST(ts6a.FuzzyEquals(ts3) == false);
  TEST(ts6b.FuzzyEquals(ts1) == false);
  TEST(ts6b.FuzzyEquals(ts3) == false);
  TEST(ts7a.FuzzyEquals(ts1) == false);
  TEST(ts7a.FuzzyEquals(ts3) == false);
  TEST(ts7b.FuzzyEquals(ts1) == false);
  TEST(ts7b.FuzzyEquals(ts3) == false);

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  qtTest testObject;

  testObject.runSuite("Constructor Tests", testCtor);
  testObject.runSuite("Modification Tests", testModification);
  testObject.runSuite("Comparison Tests", testComparison);
  testObject.runSuite("Equality Tests", testEquality);
  testObject.runSuite("Fuzzy Equality Tests", testFuzzyEquality);
  return testObject.result();
}
