// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVsTrackInfo_h
#define __vtkVsTrackInfo_h

#include <vgExport.h>

#include <vtkVgEventBase.h>

#include "vsTrackId.h"

class VSP_DATA_EXPORT vtkVsTrackInfo : public vtkVgEventTrackInfoBase
{
  vtkDeclareMetaObject(vtkVsTrackInfo);

public:
  vtkVsTrackInfo(vsTrackId tid,
                 const vtkVgTimeStamp& start,
                 const vtkVgTimeStamp& end);
  vtkVsTrackInfo(const vtkVsTrackInfo&);
  virtual ~vtkVsTrackInfo();

  virtual vtkVgEventTrackInfoBase* Clone() const;

  virtual const char* CheckValid() const;

  vsTrackId LogicalId;
};

#endif
