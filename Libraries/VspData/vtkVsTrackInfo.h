/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVsTrackInfo_h
#define __vtkVsTrackInfo_h

#include <vgExport.h>

#include <vtkVgEventBase.h>

#include <vvTrack.h>

class VSP_DATA_EXPORT vtkVsTrackInfo : public vtkVgEventTrackInfoBase
{
  vtkDeclareMetaObject(vtkVsTrackInfo);

public:
  vtkVsTrackInfo(vvTrackId tid,
                 const vtkVgTimeStamp& start,
                 const vtkVgTimeStamp& end);
  vtkVsTrackInfo(const vtkVsTrackInfo&);
  virtual ~vtkVsTrackInfo();

  virtual vtkVgEventTrackInfoBase* Clone() const;

  virtual const char* CheckValid() const;

  vvTrackId VvTrackId;
};

#endif
