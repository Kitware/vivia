/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVQLayoutManagerBase_h
#define __vtkVQLayoutManagerBase_h

// Description:
// This class will resolve position / orient nodes in some unique way.
// Depending upon whether a video is playing / stopped, area overlap
// with other vidoes, a concrete implementation would provide ways to
// locate the videos in some sensible way according to the specific algorithm.

#include <vtkObject.h>
#include <vtkSmartPointer.h>

// Forware declarations.
class vtkVgGroupNode;
class vtkVgTimeStamp;

class vtkVQLayoutManagerBase : public vtkObject
{
public:

  // Description:
  typedef vtkSmartPointer<vtkVQLayoutManagerBase> Ref;

  vtkTypeMacro(vtkVQLayoutManagerBase, vtkObject);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  const vtkVgGroupNode* GetScene() const;
  vtkVgGroupNode* GetScene();
  void SetScene(vtkVgGroupNode* root);

  // Description:
  virtual void Update(vtkVgTimeStamp* timestamp) {;}


protected:
  vtkVQLayoutManagerBase();
  virtual ~vtkVQLayoutManagerBase();

private:
  vtkVQLayoutManagerBase(const vtkVQLayoutManagerBase&); // Not implemented.
  void operator= (const vtkVQLayoutManagerBase&);        // Not implemented.
};


#endif // __vtkVQLayoutManagerBase_h
