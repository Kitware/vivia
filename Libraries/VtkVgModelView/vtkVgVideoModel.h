// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoModel_h
#define __vtkVgVideoModel_h

#include <vector>

#include <vtkSmartPointer.h>

#include <vgExport.h>

#include <vtkVgVideoFrame.h>
#include <vtkVgVideoSourceBase.h>

#include "vtkVgModelBase.h"

// Single entity that represents a video with optional child models.
class VTKVG_MODELVIEW_EXPORT vtkVgVideoModel : public vtkVgModelBase
{
public:
  vtkVgClassMacro(vtkVgVideoModel);
  vtkTypeMacro(vtkVgVideoModel, vtkVgModelBase);

  // Description:
  // Use \c New() to create an instance of \c vtkVgVideoModel
  static vtkVgVideoModel* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get \c vtkVgVideoSourceBase.
  void SetVideoSource(vtkVgVideoSourceBase* videoSource);
  vtkVgVideoSourceBase* GetVideoSource();
  const vtkVgVideoSourceBase* GetVideoSource() const;

  // Description:
  // Seek video to the specified time stamp and update child models with the
  // result of the seek.
  virtual int Update(const vtkVgTimeStamp& timeStamp,
                     const vtkVgTimeStamp* referenceFrameTimeStamp);
  using Superclass::Update;

  // Description:
  // Return current video frame.
  const vtkVgVideoFrame& GetCurrentVideoFrame() const;

  // Description:
  // Add a child model to this video model. The child model's time stamp will
  // be synchronized with the video.
  void AddChildModel(vtkVgModelBase* model);

  // Description:
  // Test if the specified model is a registered child model of this video
  // model.
  bool HasChildModel(vtkVgModelBase* model);

  // Description:
  // Return count of child models.
  size_t GetChildModelCount() const;

  // Description:
  // Return child model at specified index, or nullptr if index is not valid.
  vtkVgModelBase* GetChildModel(size_t index) const;

  // Description:
  // Remove specified child model. Return true if the model was removed, false
  // if the specified model is not a child of this video model.
  bool RemoveChildModel(vtkVgModelBase* model);

  // Description:
  // Remove child model at specified index. Return true if a model was removed,
  // false if the index is not valid.
  bool RemoveChildModel(size_t index);

  // Description:
  // Return the MTime for the last Update (that did something).
  virtual unsigned long GetUpdateTime();

protected:
  vtkVgVideoModel();
  virtual ~vtkVgVideoModel();

  void UpdateChildren(const vtkVgTimeStamp& timeStamp,
                      const vtkVgTimeStamp* referenceFrameTimeStamp = 0);

  vtkSmartPointer<vtkVgVideoSourceBase> VideoSource;
  vtkVgVideoFrame CurrentVideoFrame;

  std::vector<vtkVgModelBase*> ChildModels;

  vtkTimeStamp UpdateTime;

private:
  vtkVgVideoModel(const vtkVgVideoModel&);  // Not implemented.
  void operator=(const vtkVgVideoModel&);   // Not implemented.
};

#endif // __vtkVgVideoModel_h
