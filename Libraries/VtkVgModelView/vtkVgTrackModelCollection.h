/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgTrackModelCollection_h
#define __vtkVgTrackModelCollection_h

// VisGUI includes.
#include <vtkVgMacros.h>

// VTK includes.
#include <vtkObject.h>

#include <vgExport.h>

// Forward declarations.
class vtkVgTrack;
class vtkVgTrackModel;

class VTKVG_MODELVIEW_EXPORT vtkVgTrackModelCollection : public vtkObject
{
public:
  // Description:
  // Macro defined for SmartPtr.
  vtkVgClassMacro(vtkVgTrackModelCollection);

  // Description:
  // Macro defined for object hierarchy and internal management.
  vtkTypeMacro(vtkVgTrackModelCollection, vtkObject);

  // Description:
  // Print information about data members.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a new instance of this class.
  static vtkVgTrackModelCollection* New();

  // Description:
  // Add/Remove \c vtkVgTrack to/from known track models.
  void AddTrack(vtkVgTrack* track);
  void RemoveTrack(vtkIdType id);

  // Description:
  // Get a track give a track id.
  vtkVgTrack* GetTrack(vtkIdType id);

  // Description:
  // Provide access to the internal model.
  vtkVgTrackModel* GetInternalTrackModel();

  // Description:
  // Add/Remove \c vtkVgTrackModel to/from internal store of track models.
  void AddTrackModel(vtkVgTrackModel* trackModel);
  void RemoveTrackModel(vtkVgTrackModel* trackModel);


protected:
  vtkVgTrackModelCollection();
  virtual ~vtkVgTrackModelCollection();


private:

  class Internal;
  Internal* Implementation;

  vtkVgTrackModelCollection(const vtkVgTrackModelCollection&);  // Not implemented.
  void operator= (const vtkVgTrackModelCollection&);            // Not implemented.
};

#endif // __vtkVgTrackModelCollection_h
