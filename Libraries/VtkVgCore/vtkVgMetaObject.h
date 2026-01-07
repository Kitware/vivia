// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgMetaObject_h
#define __vtkVgMetaObject_h

#include <vgExport.h>

struct VTKVG_CORE_EXPORT vtkMetaObjectId
{
  const char* Name;
  const vtkMetaObjectId* Parent;
};

#define vtkDeclareMetaObject(_class) \
  public: \
  static _class* SafeDownCast(vtkVgMetaObject *object) \
  { \
    const vtkMetaObjectId *metaId = object->MetaId(); \
    while (metaId) \
      { \
      if (metaId == &_class::MetaObject) \
        { \
        return static_cast<_class*>(object); \
        } \
      metaId = metaId->Parent; \
      } \
    return 0; \
  } \
  virtual const vtkMetaObjectId* MetaId() \
  { \
    return &_class::MetaObject; \
  } \
  static const vtkMetaObjectId MetaObject;

#define vtkImplementMetaObject(_class, _base) \
  const vtkMetaObjectId _class::MetaObject = { #_class, &_base::MetaObject }

class VTKVG_CORE_EXPORT vtkVgMetaObject
{
  vtkDeclareMetaObject(vtkVgMetaObject)

public:
  vtkVgMetaObject() {}
  explicit vtkVgMetaObject(const vtkVgMetaObject&) {}
  virtual ~vtkVgMetaObject() {}
};

#endif // __vtkVgMetaObject_h
