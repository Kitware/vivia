// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgVideoNode_h
#define __vtkVgVideoNode_h

#include "vtkVgLeafNodeBase.h"

#include <vgExport.h>

// Forware declarations.
class vtkVgNodeVisitorBase;
class vtkVgPropCollection;
class vtkVgRepresentationBase;
class vtkVgVideoModel0;
class vtkVgVideoRepresentationBase0;

class VTKVG_SCENEGRAPH_EXPORT vtkVgVideoNode : public vtkVgLeafNodeBase
{
public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vtkVgVideoNode);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgVideoNode, vtkVgNodeBase);

  static vtkVgVideoNode* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get video relevancy score.
  vtkSetMacro(UserScore, int);
  vtkGetMacro(UserScore, int);

  // Description:
  // Set/Get video relevancy score.
  vtkSetMacro(RelevancyScore, double);
  vtkGetMacro(RelevancyScore, double);

  // Description:
  // Set/Get video preference score (for feedback results)
  vtkSetMacro(PreferenceScore, double);
  vtkGetMacro(PreferenceScore, double);

  // Description:
  // Set/Get video relevancy score.
  vtkSetMacro(InstanceId, vtkIdType);
  vtkGetMacro(InstanceId, vtkIdType);

  // Description:
  // Set/Get video stream id.
  vtkSetStringMacro(StreamId);
  vtkGetStringMacro(StreamId);

  // Description:
  // Set/Get video mission id.
  vtkSetStringMacro(MissionId);
  vtkGetStringMacro(MissionId);

  // Description:
  // Set/Get video time range.
  vtkSetVector2Macro(TimeRange, double);
  vtkGetVector2Macro(TimeRange, double);

  // Description:
  // Set/Get the color scalar for this item. This value depends on all the other
  // video items visible in the scene, and is computed externally.  This may
  // be rank or score-based depending on configuration options. 0.0 means 'hot',
  // 1.0 means 'cold', and 2.0 is special value indicating an unscored item.
  vtkSetMacro(ColorScalar, double);
  vtkGetMacro(ColorScalar, double);

  // Description:
  // Set/Get ranking of this video based on some criteria.
  vtkSetMacro(Rank, long long);
  vtkGetMacro(Rank, long long);

  // Description:
  // Set/Get whether this is a refinement result, a normal result, or both.
  vtkSetMacro(IsNormalResult, bool);
  vtkGetMacro(IsNormalResult, bool);

  vtkSetMacro(IsRefinementResult, bool);
  vtkGetMacro(IsRefinementResult, bool);

  vtkSetMacro(ActivityType, int);
  vtkGetMacro(ActivityType, int);

  vtkSetMacro(IsStarResult, bool);
  vtkGetMacro(IsStarResult, bool);

  vtkSetMacro(HasVideoData, bool);
  vtkGetMacro(HasVideoData, bool);

  // Description:
  // Set/Get video note.
  vtkSetStringMacro(Note);
  vtkGetStringMacro(Note);

  // Description:
  // Set/Get video representation.
  void SetVideoRepresentation(vtkVgVideoRepresentationBase0* videoRepresentation);
  vtkVgVideoRepresentationBase0* GetVideoRepresentation();
  const vtkVgVideoRepresentationBase0* GetVideoRepresentation() const;

  // Description:
  // Set visibility of this node.
  virtual int SetVisible(int flag);

  // Description:
  // Overridden functions.
  virtual void Update(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void UpdateRenderObjects(vtkVgPropCollection* propCollection);

  virtual void Accept(vtkVgNodeVisitorBase&  nodeVisitor);

  virtual void Traverse(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void ComputeBounds();

protected:
  vtkVgVideoNode();
  virtual ~vtkVgVideoNode();

  char*     StreamId;
  char*     MissionId;
  vtkIdType InstanceId;
  double    TimeRange[2];

  int       UserScore;
  double    RelevancyScore;
  double    PreferenceScore;
  double    ColorScalar;
  long long Rank;

  bool      IsNormalResult;
  bool      IsRefinementResult;

  bool      IsStarResult;
  bool      HasVideoData;

  int       ActivityType;

  char* Note;

  vtkSmartPointer<vtkVgVideoModel0>              VideoModel;
  vtkSmartPointer<vtkVgVideoRepresentationBase0> VideoRepresentation;

private:
  vtkVgVideoNode(const vtkVgVideoNode&); // Not implemented.
  void operator=(const vtkVgVideoNode&); // Not implemented.
};

#endif // __vtkVgVideoNode_h
