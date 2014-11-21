/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
