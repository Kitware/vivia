// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __TestObject_h
#define __TestObject_h

#include <vtkObject.h>

class TestObject : public vtkObject
{
public:
  static TestObject* New() { return new TestObject; }
  vtkTypeMacro(TestObject, vtkObject);

  int GetData() const { return this->Data; }
  void SetData(int n) { this->Data = n; }

  static void SetCounter(int* c) { TestObject::Counter = c; }
  void SetFlag(bool* f) { this->Flag = f; f && (*f = true); }

  void DeepCopy(TestObject* src) { this->Data = src->Data; }

protected:
  TestObject() : Data(0), Flag(0)
    {
    this->Counter && ++(*this->Counter);
    }

  virtual ~TestObject()
    {
    this->Counter && --(*this->Counter);
    this->Flag && (*this->Flag = false);
    }

  int Data;
  bool* Flag;
  static int* Counter;
};

int* TestObject::Counter = 0;

class TestCounter
{
public:
  TestCounter() : Value(0) { TestObject::SetCounter(&this->Value); }
  ~TestCounter() { TestObject::SetCounter(0); }
  int GetValue() const { return this->Value; }

protected:
  int Value;
};

#endif
