// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqUserActions_h
#define __vqUserActions_h

// QT includes.
#include <QObject>

// Forward declarations.
class vqCore;

class vtkCamera;
class vtkRenderer;

class vtkVgNodeBase;
class vtkVgVideoNode;

class vqUserActions : public QObject
{
  Q_OBJECT
public:
  vqUserActions(vqCore* core);
  virtual ~vqUserActions();

signals:
  void UpdateView();
  void Play(vtkVgVideoNode&);
  void Pause(vtkVgVideoNode&);
  void Stop(vtkVgVideoNode&);
  void Prev(vtkVgVideoNode&);
  void Next(vtkVgVideoNode&);
  void Select(vtkVgVideoNode&);

public slots:
  void Select(QList<vtkVgNodeBase*> nodes);
  void Activate(vtkVgNodeBase& node);
  void Reset();

  void FocusTo(vtkVgNodeBase& node);
  void FocusTo(vtkVgVideoNode& node);

  void CenterAndZoomTo(vtkVgNodeBase& node,  bool alwaysZoom = false);
  void CenterAndZoomTo(vtkVgVideoNode& node, bool alwaysZoom = false);
  void UnZoom();

  void ScrollForward(vtkVgNodeBase& node, bool* eventHandled);
  void ScrollBackward(vtkVgNodeBase& node, bool* eventHandled);

  void SetAutoZoom(bool enable);
  bool GetAutoZoom() const;

private:

  void CalculateBoundsAndCenter(
    vtkVgNodeBase& node, double bounds[6], double center[2]);

  bool CheckIfFullyContained2D(double nodeBounds[6], double worldBounds[6]);
  bool CheckIfFullyContained(double nodeBounds[6], vtkRenderer* renderer);

  void DeselectRecursive(vtkVgNodeBase* root);
  void TogglePlayPause(vtkVgVideoNode& node);

private:
  vqCore*         CoreInstance;
  vtkVgVideoNode* LastActiveVideoNode;
  vtkVgNodeBase*  LastZoomedNode;
  vtkCamera*      PreZoomCamera;

  bool IsZoomed;
  bool AutoZoom;
};

#endif // __vqUserActions_h
