// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgAbstractFactory_h
#define __vgAbstractFactory_h

#include <vector>

//-----------------------------------------------------------------------------
template <typename Type, typename Key>
class vgAbstractFactory
{
public:

  typedef Type(*Creator)();

  virtual Type Create(Key key) = 0;
  virtual bool Register(Creator function)
    {
    if (function)
      {
      this->Creators.push_back(function);
      return true;
      }
    else
      {
      return false;
      }
    }

  virtual ~vgAbstractFactory()
    {
    }

protected:

  std::vector<Creator> Creators;
};

#endif // __vgAbstractFactory_h
