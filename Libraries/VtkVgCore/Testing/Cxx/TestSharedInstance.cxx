// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <qtTest.h>

#include "vtkVgSharedInstance.h"

#include "TestObject.h"

typedef vtkVgSharedInstance<TestObject> Instance;

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
int testAssignmentCtor(qtTest& testObject)
{
  bool flag;
  TestObject* p = TestObject::New();
  p->SetFlag(&flag);

  // Set some data on the object
  p->SetData(3);
  TEST_EQUAL(p->GetData(), 3);

    {
    // Test construction
    Instance i(p);
    TEST_EQUAL(i.GetVolatilePointer(), p);
    TEST_EQUAL(i->GetData(), 3);
    }

  // Verify object was destroyed
  TEST_EQUAL(flag, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testSharing(qtTest& testObject)
{
  TestCounter counter;

  Instance i1, i2;
  TEST_EQUAL(counter.GetValue(), 2);   // i1, i2
  TEST(i1.GetVolatileConstPointer() != i2.GetVolatileConstPointer());
  i1->SetData(1);
  i2->SetData(2);

  // Test copy is shared
  Instance i3(i1);
  const Instance& ci3 = i3;
  TEST_EQUAL(counter.GetValue(), 2);   // i1 == i3, i2
  TEST(&i3 == &ci3);
  TEST_EQUAL(ci3.GetVolatileConstPointer(), i1.GetVolatileConstPointer());
  TEST_EQUAL(ci3->GetData(), 1);
  TEST_EQUAL(i3.GetVolatileConstPointer(), i1.GetVolatileConstPointer());

  // Test assignment is shared
  Instance i4;
  const Instance& ci4 = i4;
  TEST_EQUAL(counter.GetValue(), 3);   // i1 == i3, i2, i4
  i4 = i2;
  TEST_EQUAL(counter.GetValue(), 2);   // i1 == i3, i2 == i4
  TEST(&i4 == &ci4);
  TEST_EQUAL(ci4.GetVolatileConstPointer(), i2.GetVolatileConstPointer());
  TEST_EQUAL(ci4->GetData(), 2);
  TEST_EQUAL(i4.GetVolatileConstPointer(), i2.GetVolatileConstPointer());

  // Test detach on write
  const TestObject* p = i1.GetVolatileConstPointer();
  Instance i5(i1);
  TEST_EQUAL(counter.GetValue(), 2);   // i1 == i3 == i5, i2 == i4
  TEST_EQUAL(i5.GetVolatileConstPointer(), p);
  i5->SetData(5);
  TEST_EQUAL(counter.GetValue(), 3);   // i1 == i3, i2 == i4, i5
  TEST_EQUAL(i1.GetVolatileConstPointer(), p);
  TEST_EQUAL(i3.GetVolatileConstPointer(), p);
  TEST(i5.GetVolatileConstPointer() != p);
  TEST_EQUAL(ci3->GetData(), 1);
  TEST_EQUAL(i5->GetData(), 5);

  // Test detach on non-const pointer access
  // NOTE: This also verifies that the underlying object pointer is indeed
  //       volatile
  TEST_EQUAL(i1.GetVolatileConstPointer(), p);
  TEST(i1.GetVolatilePointer() != p);
  TEST_EQUAL(counter.GetValue(), 4);   // i1, i2 == i4, i3, i5
  TEST(i1.GetVolatileConstPointer() != p);

  return 0;
}

//-----------------------------------------------------------------------------
int testReset(qtTest& testObject)
{
  TestCounter counter;
  bool flag;

  // Create an object
  TestObject* p = TestObject::New();
  p->SetFlag(&flag);
  TEST_EQUAL(counter.GetValue(), 1);

  // Set some data on the object
  p->SetData(8);
  TEST_EQUAL(p->GetData(), 8);

    {
    // Create an instance
    Instance i;
    TEST_EQUAL(counter.GetValue(), 2);

    // Assign the first object to the instance
    i.Reset(p);
    TEST_EQUAL(counter.GetValue(), 1);
    TEST_EQUAL(i->GetData(), 8);
    TEST_EQUAL(i.GetVolatilePointer(), p);
    }

  // Verify the object was destroyed
  TEST_EQUAL(counter.GetValue(), 0);
  TEST_EQUAL(flag, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testAssumeOwnership(qtTest& testObject)
{
  bool flag;
  TestObject* p = TestObject::New();
  const TestObject* cp = p;
  p->SetFlag(&flag);
  Instance i1(p);

  // Set some data on the object
  i1->SetData(7);
  TEST_EQUAL(i1->GetData(), 7);
  TEST_EQUAL(i1.GetVolatilePointer(), p);

    {
    // Create a second instance sharing the first
    Instance i2(i1);
    TEST_EQUAL(i2.GetVolatileConstPointer(), cp);

    // Modify the first instance so that it detaches
    i1->SetData(11);
    TEST(i1.GetVolatilePointer() != p);
    }

  // Verify the original object was destroyed
  TEST_EQUAL(flag, false);

  return 0;
}

//-----------------------------------------------------------------------------
int testPoison(qtTest& testObject)
{
  TestCounter counter;

  // Create first instance
  Instance i1;
  i1->SetData(6);
  TEST_EQUAL(counter.GetValue(), 1);
  TEST_EQUAL(i1->GetData(), 6);

  // Poison first instance
  const TestObject* p1 = i1.GetPointer();

  // Create second instance as a copy of the first
  const Instance i2(i1);
  TEST_EQUAL(i2->GetData(), 6);

  // Verify that second instance is a deep copy...
  TEST_EQUAL(counter.GetValue(), 2);
  TEST(i2.GetVolatileConstPointer() != p1);

  // ...and first instance did not change
  const TestObject* p = i1.GetPointer();
  TEST_EQUAL(p, p1);

  // Release non-volatile pointer to first instance
  i1.ReleasePointer();

  // Verify that copies now are shared
  const Instance i3(i1);
  TEST_EQUAL(counter.GetValue(), 2);
  TEST_EQUAL(i3.GetVolatileConstPointer(), p1);
  TEST_EQUAL(i3->GetData(), 6);

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  qtTest testObject;

  testObject.runSuite("Default Constructor",           testDefaultCtor);
  testObject.runSuite("Assignment Constructor",        testAssignmentCtor);
  testObject.runSuite("Sharing",                       testSharing);
  testObject.runSuite("Reset Method",                  testReset);
  testObject.runSuite("Assumption of Ownership",       testAssumeOwnership);
  testObject.runSuite("Poisoning",                     testPoison);
  return testObject.result();
}
