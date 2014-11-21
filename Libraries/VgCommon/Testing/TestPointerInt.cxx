/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <qtTest.h>

#include "../vgPointerInt.h"

class A
{
  int n;

public:
  A() {}
};

A* a1;
A* a2;

enum
{
  BitOne   = 1 << 0,
  BitTwo   = 1 << 1,
  BitThree = 1 << 2,
  BitOneAndThree = BitOne | BitThree
};

typedef vgPointerInt<A*> AP;

//-----------------------------------------------------------------------------
int testCtors(qtTest& testObject)
{
  {
  AP v;
  TEST(v.GetPointer() == 0);
  TEST(v.GetInt() == 0);
  }

  {
  AP v(a1);
  TEST(v.GetPointer() == a1);
  TEST(v.GetInt() == 0);
  }

  {
  AP v(a1, BitOneAndThree);
  TEST(v.GetPointer() == a1);
  TEST(v.GetInt() == BitOneAndThree);
  }

  {
  AP v(0, BitTwo);
  TEST(v.GetPointer() == 0);
  TEST(v.GetInt() == BitTwo);
  }

  return 0;
}

//-----------------------------------------------------------------------------
int testInteger(qtTest& testObject)
{
  {
  AP v;
  v.SetInt(BitOneAndThree);
  TEST(v.GetInt() == BitOneAndThree);
  TEST(v.GetPointer() == 0);
  }

  {
  AP v;
  v.SetPointer(a1);
  TEST(v.GetPointer() == a1);
  TEST(v.GetInt() == 0);
  }

  {
  AP v;
  v.SetPointer(a1);
  v.SetPointer(0);
  TEST(v.GetPointer() == 0);
  TEST(v.GetInt() == 0);
  }

  {
  AP v;
  v.SetPointer(a1);
  v.SetInt(BitOneAndThree);
  TEST(v.GetPointer() == a1);
  TEST(v.GetInt() == BitOneAndThree);
  }

  {
  AP v;
  v.SetInt(BitOneAndThree);
  v.SetPointer(a1);
  TEST(v.GetInt() == BitOneAndThree);
  TEST(v.GetPointer() == a1);
  }

  {
  AP v;
  v.SetPointer(a1);
  v.SetPointer(a2);
  TEST(v.GetPointer() == a2);
  }

  return 0;
}

//-----------------------------------------------------------------------------
int testBitwise(qtTest& testObject)
{
  {
  AP v;
  v.SetBits(BitOneAndThree);
  TEST(v.GetBitsAreSet(BitOneAndThree));
  TEST(v.GetBitsAreSet(BitOne));
  TEST(v.GetBitsAreSet(BitThree));
  TEST(!v.GetBitsAreSet(BitTwo));
  TEST(!v.GetBitsAreSet(BitOne | BitTwo | BitThree));
  }

  {
  AP v;
  v.SetBits(BitOne);
  v.SetBits(BitTwo);
  v.SetBits(BitThree);
  TEST(v.GetBitsAreSet(BitOne));
  TEST(v.GetBitsAreSet(BitTwo));
  TEST(v.GetBitsAreSet(BitThree));
  TEST(v.GetBitsAreSet(BitOne | BitTwo | BitThree));
  TEST(v.GetInt() == (BitOne | BitTwo | BitThree));
  }

  {
  AP v;
  v.SetPointer(a1);
  v.SetBits(BitOneAndThree);
  TEST(v.GetBitsAreSet(BitOneAndThree));
  TEST(v.GetPointer() == a1);
  }

  {
  AP v;
  v.SetBits(BitOneAndThree);
  v.SetPointer(a1);
  TEST(v.GetPointer() == a1);
  TEST(v.GetBitsAreSet(BitOneAndThree));
  }

  {
  AP v;
  v.SetBits(BitOneAndThree);
  v.SetPointer(a1);
  v.ClearBits(BitTwo);
  TEST(v.GetBitsAreSet(BitOneAndThree));
  TEST(v.GetPointer() == a1);
  v.ClearBits(BitOne);
  TEST(!v.GetBitsAreSet(BitOne));
  TEST(v.GetBitsAreSet(BitThree));
  v.ClearBits(BitThree);
  TEST(!v.GetBitsAreSet(BitThree));
  TEST(v.GetInt() == 0);
  TEST(v.GetPointer() == a1);
  }

  {
  AP v;
  v.SetBits(BitOneAndThree);
  v.SetPointer(a1);
  TEST(v.GetBitsAreSet(BitOne));
  TEST(!v.GetBitsAreSet(BitTwo));
  v.ClearBits(BitThree);
  TEST(!v.GetBitsAreSet(BitThree));
  TEST(v.GetPointer() == a1);
  }

  return 0;
}

//-----------------------------------------------------------------------------
A* makeTestPointer()
{
  // Make sure the upper words of a 64-bit pointer have something in them.
  // Otherwise we may miss some errors that only occur with pointers above
  // the 4 GB boundary.
  A* a = new A;
  if (sizeof(a) == 8 && uintptr(a) < (1ULL << 32))
    {
    a = (A*)(uintptr(a) | (uintptr(a) << 32));
    }
  return a;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  // Some memory will leak here, but we don't really care.
  a1 = makeTestPointer();
  a2 = makeTestPointer();

  qtTest testObject;

  testObject.runSuite("Constructor Tests", testCtors);
  testObject.runSuite("Integer Operation Tests", testInteger);
  testObject.runSuite("Bitwise Operation Tests", testBitwise);

  return testObject.result();
}
