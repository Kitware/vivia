// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <qtTest.h>

#include "vtkVgInstance.h"

#include "TestObject.h"

typedef vtkVgInstance<TestObject> Instance;

//-----------------------------------------------------------------------------
int testDefaultCtor(qtTest& testObject)
{
  bool flag = false;
    {
    // Test construction
    Instance i;
    TEST_EQUAL(i->GetData(), 0);

    // Test basic access
    i->SetFlag(&flag);
    TEST_EQUAL(flag, true);

    i->SetData(5);
    TEST_EQUAL(i->GetData(), 5);
    }

  // Verify object was destroyed
  TEST_EQUAL(flag, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testCopyCtor(qtTest& testObject)
{
  bool flag1, flag2;
    {
    Instance i1;
    TestObject* p1 = i1;
    i1->SetFlag(&flag1);

    // Set some data on first instance
    i1->SetData(4);
    TEST_EQUAL(i1->GetData(), 4);

      {
      // Test construction
      Instance i2(i1);
      TestObject* p2 = i2;
      i2->SetFlag(&flag2);

      // Verify copy
      TEST(p2 != p1);
      TEST(p2 != i1);
      TEST_EQUAL(i2->GetData(), 4);
      }
    }

  // Verify objects were destroyed
  TEST_EQUAL(flag1, false);
  TEST_EQUAL(flag2, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testAssignment(qtTest& testObject)
{
  bool flag1, flag2;
    {
    Instance i1;
    TestObject* p1 = i1;
    i1->SetFlag(&flag1);
    TEST_EQUAL(i1->GetData(), 0);

      {
      // Test construction
      Instance i2;
      TestObject* p2 = i2;
      i2->SetFlag(&flag2);

      // Set some data on second instance
      i2->SetData(3);
      TEST_EQUAL(i2->GetData(), 3);

      // Copy instance to first instance
      i1 = i2;
      TEST_EQUAL(i1->GetData(), 3);

      // Set some different data on second instance
      i2->SetData(7);
      TEST_EQUAL(i2->GetData(), 7);

      // Copy pointer to first instance
      i1 = p2;
      TEST_EQUAL(i1->GetData(), 7);
      }

    // Verify object was destroyed
    TEST_EQUAL(flag2, false);

    // Verify that assignment did not change the underlying object
    TestObject* p = i1;
    TEST_EQUAL(p, p1);
    TEST_EQUAL(flag1, true);
    TEST_EQUAL(i1->GetData(), 7);
    }

  // Verify object was destroyed
  TEST_EQUAL(flag1, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testPtrCtor1(qtTest& testObject)
{
  bool flag;
  TestObject* p = TestObject::New();
  p->SetFlag(&flag);

  // Set some data on the object
  p->SetData(9);
  TEST_EQUAL(p->GetData(), 9);

    {
    // Test construction
    Instance i(p, Instance::TakeReference);

    // Verify construction
    TestObject* pi = i;
    TEST_EQUAL(pi, p);
    TEST_EQUAL(i->GetData(), 9);
    }

  // Verify object was NOT destroyed
  TEST_EQUAL(flag, true);
  TEST_EQUAL(p->GetData(), 9);

    {
    // Test construction (take 2)
    Instance i(p, Instance::TakeReference);

    // Verify construction (take 2)
    TestObject* pi = i;
    TEST_EQUAL(pi, p);
    TEST_EQUAL(i->GetData(), 9);

    // Verify object still valid after releasing original reference
    p->Delete();
    TEST_EQUAL(flag, true);
    TEST_EQUAL(p->GetData(), 9);
    }

  // Verify object is destroyed
  TEST_EQUAL(flag, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testPtrCtor2(qtTest& testObject)
{
  bool flag;
  TestObject* p = TestObject::New();
  p->SetFlag(&flag);

  // Set some data on the object
  p->SetData(6);
  TEST_EQUAL(p->GetData(), 6);

    {
    // Test construction
    Instance i(p, Instance::TakeOwnership);

    // Verify construction
    TestObject* pi = i;
    TEST_EQUAL(pi, p);
    TEST_EQUAL(i->GetData(), 6);
    }

  // Verify object was destroyed
  TEST_EQUAL(flag, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testPtrCtor3(qtTest& testObject)
{
  bool flag1, flag2;
  TestObject* p1 = TestObject::New();
  p1->SetFlag(&flag1);

  // Set some data on first object
  p1->SetData(2);
  TEST_EQUAL(p1->GetData(), 2);

    {
    // Test construction
    Instance i2(p1, Instance::CopyByAssignment);
    TestObject* p2 = i2;
    i2->SetFlag(&flag2);

    // Verify copy
    TEST(p2 != p1);
    TEST_EQUAL(i2->GetData(), 2);

    // Verify instance valid after first object deleted
    p1->Delete();
    TEST_EQUAL(flag1, false);
    TEST_EQUAL(flag2, true);
    TEST_EQUAL(i2->GetData(), 2);
    }

  // Verify object was destroyed
  TEST_EQUAL(flag2, false);

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  qtTest testObject;

  testObject.runSuite("Default Constructor",               testDefaultCtor);
  testObject.runSuite("Copy Constructor",                  testCopyCtor);
  testObject.runSuite("Assignment Operator",               testAssignment);
  testObject.runSuite("Take-Reference Constructor",        testPtrCtor1);
  testObject.runSuite("Take-Ownership Constructor",        testPtrCtor2);
  testObject.runSuite("Copy-by-Assignment Constructor",    testPtrCtor3);
  return testObject.result();
}
