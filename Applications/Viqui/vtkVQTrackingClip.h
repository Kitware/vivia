/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVQTrackingClip_h
#define __vtkVQTrackingClip_h

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

class vtkImageData;
class vtkVgEvent;
class vtkVgVideoNode;
class vtkVgVideoProviderBase;

class vtkVQTrackingClip : public vtkObject
{
public:
  vtkTypeMacro(vtkVQTrackingClip, vtkObject);
  static vtkVQTrackingClip* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  void SetVideo(vtkVgVideoProviderBase* vs);
  void SetEvent(vtkVgEvent* event);

  void SetVideoNode(vtkVgVideoNode* node);
  vtkVgVideoNode* GetVideoNode();

  vtkSetMacro(Rank, int);
  vtkGetMacro(Rank, int);

  vtkSetVectorMacro(Dimensions, int, 2);
  vtkGetVectorMacro(Dimensions, int, 2);

  vtkImageData* GetOutputImageData();

  double GetDuration();

  vtkIdType GetEventId();

  void Update();

protected:
  vtkVQTrackingClip();
  virtual ~vtkVQTrackingClip();

private:
  vtkVQTrackingClip(const vtkVQTrackingClip&); // Not implemented.
  void operator=(const vtkVQTrackingClip&);    // Not implemented.

  vtkSmartPointer<vtkVgVideoProviderBase> Video;
  vtkSmartPointer<vtkVgVideoNode>       VideoNode;
  vtkSmartPointer<vtkVgEvent>           Event;

  int Rank;

  vtkSmartPointer<vtkImageData>         OutputImageData;

  int Dimensions[2];

  vtkTimeStamp UpdateTime;
};

#endif // __vtkVQTrackingClip_h
