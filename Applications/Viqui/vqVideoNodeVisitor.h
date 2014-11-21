#ifndef VQVIDEONODEVISITOR_H
#define VQVIDEONODEVISITOR_H

#include <vtkVgNodeVisitor.h>
#include <vtkVgVideoNode.h>
#include <vtkVgVideoRepresentation0.h>

//-----------------------------------------------------------------------------
struct vqVideoNodeVisitor : public vtkVgNodeVisitor
{
  vqVideoNodeVisitor() : vtkVgNodeVisitor(),
    VideoClipVisible(true),
    UpdateVideoClipVisibility(true),
    VideoOutlineVisible(true),
    UpdateVideoOutlineVisibility(true)
    {
    }

  virtual void Visit(vtkVgGeode& geode)
    {
    vtkVgNodeVisitor::Visit(geode);
    }

  virtual void Visit(vtkVgGroupNode&  groupNode)
    {
    vtkVgNodeVisitor::Visit(groupNode);
    }

  virtual void Visit(vtkVgTransformNode& transformNode)
    {
    vtkVgNodeVisitor::Visit(transformNode);
    }


  virtual void Visit(vtkVgLeafNodeBase&  leafNode)
    {
    vtkVgNodeVisitor::Visit(leafNode);
    }


  virtual void Visit(vtkVgVideoNode& videoNode)
    {
    vtkVgVideoRepresentation0::SmartPtr videoRepresentation =
      dynamic_cast<vtkVgVideoRepresentation0*>(videoNode.GetVideoRepresentation());
    if (videoRepresentation)
      {
      if (this->UpdateVideoClipVisibility)
        {
        videoRepresentation->SetVideoVisible(this->VideoClipVisible);
        }

      if (this->UpdateVideoOutlineVisibility)
        {
        videoRepresentation->SetOutlineVisible(this->VideoOutlineVisible);
        }
      }
    }

  bool VideoClipVisible;
  bool UpdateVideoClipVisibility;
  bool VideoOutlineVisible;
  bool UpdateVideoOutlineVisibility;
};

#endif // VQVIDEONODEVISITOR_H
