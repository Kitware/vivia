// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVQBlastLayoutNode_h
#define __vtkVQBlastLayoutNode_h

// QT includes.
#include <QObject>

// VG includes.
#include "vtkVgMacros.h"
#include "vtkVgGroupNode.h"

class vtkVgVideoNode;
class vtkVgSelectionWidget;

class vtkVQBlastLayoutNode : public QObject, public vtkVgGroupNode
{
  Q_OBJECT

public:
  enum LayoutModeType
    {
    Z_SORT = 0,
    BLAST,
    STACK
    };

  vtkVgClassMacro(vtkVQBlastLayoutNode);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVQBlastLayoutNode, vtkVgGroupNode)

  static vtkVQBlastLayoutNode* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get layout mode.
  void SetLayoutMode(int);
  vtkGetMacro(LayoutMode, int);

  // Description:
  // Set/Get blast flag that will move the children away from each other.
  vtkSetMacro(Blast, int);
  vtkGetMacro(Blast, int);
  vtkBooleanMacro(Blast, int);

  // Description:
  vtkSetMacro(Stack, int);
  vtkGetMacro(Stack, int);
  vtkBooleanMacro(Stack, int);

  // Description:
  vtkSetMacro(ZSort, int);
  vtkGetMacro(ZSort, int);
  vtkBooleanMacro(ZSort, int);

  // Description:
  // Set/Get factor to offset node based on their relevancy score (for ZSort).
  // Defaults to 1.1.
  vtkSetMacro(ZSortOffsetFactor, double);
  vtkGetMacro(ZSortOffsetFactor, double);

  // Description:
  // Add nodes that are present in the scene and needs to be
  // considered for the layout (eg. to avoid collision).
  void AddSceneNode(vtkVgNodeBase* node);

  // Description:
  // Overridden functions.
  virtual void Update(vtkVgNodeVisitorBase& nodeVisitor);

  virtual void Traverse(vtkVgNodeVisitorBase& nodeVisitor);

  void UpdateStackLayout();

  vtkGetObjectMacro(StackSelectionWidget, vtkVgSelectionWidget);

  void MoveToSelectedStackItem();

  vtkGetMacro(NodeFocused, int);

signals:

  void VisibilityChanged(vtkVgNodeBase&);

public slots:

  // Description:
  // Handle node picking.
  void OnPlayStop(vtkVgVideoNode& node);

  // Description:
  // Handle node selection... if STACK, change to next in stack
  void OnNextInStack(vtkVgNodeBase& node);

  // Description:
  // Check if two bounds are exactly* overlapping.
  bool IsCoincidental(double bounds1[6], double bounds2[6]);

  // Description:
  // If current representation is STACK, build (rebuild) the bar
  void BuildStackRepresentationBar();

  // Description:
  // If current representation is STACK, update the location (and size) of the bar
  void UpdateStackRepresentationBarLocation();

protected:
  vtkVQBlastLayoutNode();
  ~vtkVQBlastLayoutNode();

  void FocusNode(vtkVgVideoNode* node);
  void RestoreFocusedNodePosition();
  void SetChildrenVisible(int visible);

  void SetLayoutToBlast(vtkVgNodeVisitorBase& nodeVisitor);
  void SetLayoutToStack(vtkVgNodeVisitorBase& nodeVisitor);
  void SetLayoutToZSort(vtkVgNodeVisitorBase& nodeVisitor);

  void UndoZSortLayout();
  void UndoStackLayout();
  void UndoBlastLayout();

  void MoveToNextVisibleNodeInStack();

  void UpdateStackCurrentItemIndicator();

  int             LayoutMode;

  int             Blast;

  int             ZSort;

  int             Stack;

  int             NodeFocused;

  double          ZSortOffsetFactor;

  static const double
  MinDelta;

  class vtkInternal;
  vtkInternal* Implementation;

  vtkVgSelectionWidget* StackSelectionWidget;

private:
  vtkVQBlastLayoutNode(const vtkVQBlastLayoutNode&); // Not implemented.
  void operator=(const vtkVQBlastLayoutNode&);       // Not implemented.
};

#endif // __vtkVQBlastLayoutNode_h
