// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vpQtViewer3d_h
#define __vpQtViewer3d_h

// VisGUI includes
#include <vtkVgPicker.h>
#include <vtkVgTimeStamp.h>

#include <qtGlobal.h>

// Qt includes
#include <QList>
#include <QObject>

// VTK includes
#include <vtkSmartPointer.h>

// Forward declarations.
class vpFileDataSource;
class vpProject;

class vtkImageActor;
class vtkImageData;
class vtkInteractorStyle;
class vtkProp;
class vtkRenderer;
class vtkRenderWindow;

class vtkVgBaseImageSource;
class vtkVgGraphRepresentation;
class vtkVgPicker;
class vtkVgRepresentationBase;

class vtkEventQtSlotConnect;

class vpViewCore;

class vpQtViewer3d : public QObject
{
  Q_OBJECT

public:
  vpQtViewer3d(QObject* parent);

  virtual ~vpQtViewer3d();

  vtkRenderer*        getSceneRenderer() const;

  void                setSceneRenderWindow(vtkRenderWindow* renderWindow);
  vtkRenderWindow*    getSceneRenderWindow() const;

  void                setInteractorStyle(vtkInteractorStyle* style);
  vtkInteractorStyle* getInteractorStyle() const;

  void                setPicker(vtkVgPicker* picker);
  vtkVgPicker*        getPicker() const;

  void                setViewCoreInstance(vpViewCore* core);
  vpViewCore*         getViewCoreInstance();

  void                setContextDataSource(vpFileDataSource* source);

  vtkVgBaseImageSource* getContextSource() const;

  void                setProject(vpProject* project);

  void                addRepresentation(vtkVgRepresentationBase* representation);

  virtual void        update(const vtkVgTimeStamp& timestamp);

  virtual void        forceRender();

  virtual void        reset();

  virtual void        pick();

  virtual void        getGraphEdgeColorModes(QStringList& list);
  virtual void        getGraphEdgeThicknessModes(QStringList& list);

signals:
  void         contextCreated();

public slots:
  virtual void onMiddleClick();

  virtual void onKeyPress();

  virtual void onInteraction();

  virtual void onResetCamera();

  virtual void resetCamera();

  virtual void resetToAOI();

  virtual void setGraphEdgeColorMode(int mode);
  virtual int  getGraphEdgeColorMode();

  virtual void setGraphEdgeThicknessMode(int mode);
  virtual int  getGraphEdgeThicknessMode();

  virtual void setGraphNodeHeightMode(int mode);
  virtual int  getGraphNodeHeightMode();

  virtual void setContextLevelOfDetailFactor(int level);

  void         updateContext(const vtkVgTimeStamp& timestamp);

  void         refreshContext();

private:
  int  calculateContextLevelOfDetail(double visibleExtents[6]);

  void getViewClippedWorldExtents(double points[][4], int numberOfPoints,
                                  double worldExtents[6]);

  vtkSmartPointer<vtkImageActor>      ContextActor;
  vpFileDataSource*                   ContextDataSource;

  QList<vtkSmartPointer<vtkVgRepresentationBase> > Representations;

  vtkSmartPointer<vtkRenderer>        SceneRenderer;
  vtkSmartPointer<vtkRenderWindow>    SceneRenderWindow;

  vtkSmartPointer<vtkInteractorStyle> InteractorStyle;

  vtkSmartPointer<vtkVgPicker>        Picker;

  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;

  vpViewCore*                         CoreInstance;

  vtkSmartPointer<vtkVgGraphRepresentation> GraphRepresentation;

  vpProject*                          Project;

  class vpQtViewer3dInternal;
  vpQtViewer3dInternal* const        Internal;

private:
  Q_DISABLE_COPY(vpQtViewer3d);
};

#endif // __vpQtViewer3d_h
