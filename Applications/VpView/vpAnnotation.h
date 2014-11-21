/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpAnnotation_h
#define __vpAnnotation_h

class vtkMatrix4x4;
class vtkProp;

class vpViewCore;
class vtkVgAnnotationActor;
class vtkVgTrack;
class vtkVgEvent;
class vtkVgActivity;

#include <vtkSmartPointer.h>

// for std::ostream
#include <iosfwd>

class vpAnnotation
{
public:
  // Description:
  // Constructor / Destructor.
  vpAnnotation();
  ~vpAnnotation();

  // Description:
  // Get the annotation actor;
  vtkProp* GetProp();

  // Description:
  // Set the object to be annotated.
  void SetAnnotatedTrack(vtkVgTrack* track);
  void SetAnnotatedEvent(vtkVgEvent* event);
  void SetAnnotatedActivity(vtkVgActivity* activity);
  void SetAnnotatedSceneElement(vtkVgTrack* track, vtkIdType id);

  // Description:
  // Set matrix used to transform object positions.
  void SetRepresentationMatrix(vtkMatrix4x4* matrix);
  vtkMatrix4x4* GetRepresentationMatrix();

  void SetViewCoreInstance(vpViewCore* vc)    { this->ViewCoreInstance = vc; }

  // Description:
  // Update the annotation.
  void Update();

  // Description:
  // Set whether annotation should be displayed.
  void SetVisible(bool visible) { this->Visible = visible; }
  bool GetVisible()             { return this->Visible; }

private:
  vpAnnotation(const vpAnnotation& src);    // Not implemented.
  void operator=(const vpAnnotation& src);  // Not implemented.

  bool UpdateTrackAnnotation(vtkVgTrack* track, const char* prefix);
  bool UpdateEventAnnotation(vtkVgEvent* event);
  bool UpdateActivityAnnotation(vtkVgActivity* event);

  void AddFrameDelta(std::ostream& os,
                     unsigned int currFrame,
                     unsigned int firstFrame, unsigned int lastFrame);

private:
  bool Visible;
  int ObjectType;
  vtkIdType ObjectId;

  union
    {
    vtkVgTrack*    Track;
    vtkVgEvent*    Event;
    vtkVgActivity* Activity;
    } Object;

  vpViewCore*        ViewCoreInstance;

  vtkVgAnnotationActor* Actor;

  vtkSmartPointer<vtkMatrix4x4> RepresentationMatrix;
};

#endif // __vpAnnotation_h
