/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpQtViewer3d.h"

// Qt includes
#include <QDebug>
#include <QStringList>

// VTK includes
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkHardwareSelector.h>
#include <vtkIdTypeArray.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkType.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>

#include "vpFileDataSource.h"
#include "vpImageSourceFactory.h"
#include "vpProject.h"
#include "vpViewCore.h"

//  VisGUI includes
#include <vtkVgPicker.h>

#include <qtStlUtil.h>

#include <vtkVgGraphModel.h>
#include <vtkVgImageSource.h>
#include <vtkVgGraphRepresentation.h>
#include <vtkVgImageSource.h>
#include <vtkVgRendererUtils.h>
#include <vtkVgPicker.h>
#include <vtkVgRepresentationBase.h>
#include <vtkVgSpaceConversion.h>

// C includes
#include <cmath>

// C++ includes
#include <limits>

class vpQtViewer3d::vpQtViewer3dInternal
{
public:

  bool                                  Initialized;

  double                                ContextSourceLODFactor;

  vtkSmartPointer<vtkImageData>         ContextData;
  vtkSmartPointer<vtkVgBaseImageSource> ContextSource;
  vtkSmartPointer<vtkPlaneSource>       ContextPlane;
  vtkSmartPointer<vtkActor>             ContextPlaneActor;

  vtkSmartPointer<vtkHardwareSelector>  Selector;
};

//-----------------------------------------------------------------------------
vpQtViewer3d::vpQtViewer3d(QObject* parent) : QObject(parent),
  Internal(new vpQtViewer3dInternal())
{
  this->ContextActor  = vtkSmartPointer<vtkImageActor>::New();

  this->ContextDataSource = 0;

  this->SceneRenderer = vtkSmartPointer<vtkRenderer>::New();

  this->SceneRenderWindow = 0;

  this->InteractorStyle   = 0;

  this->Picker = vtkSmartPointer<vtkVgPicker>::New();
  this->Picker->SetVerbose(0);

  this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();

  this->CoreInstance = 0;

  this->GraphRepresentation = 0;

  this->Project = 0;

  this->Internal->Initialized = false;

  this->Internal->ContextSourceLODFactor = 1.0;

  this->Internal->ContextData       = vtkSmartPointer<vtkImageData>::New();
  this->Internal->ContextPlane      = vtkSmartPointer<vtkPlaneSource>::New();
  this->Internal->ContextPlaneActor = vtkSmartPointer<vtkActor>::New();
  this->Internal->ContextSource     = 0;

  this->Internal->Selector          = 0;
}

//-----------------------------------------------------------------------------
vpQtViewer3d::~vpQtViewer3d()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
vtkRenderer* vpQtViewer3d::getSceneRenderer() const
{
  return this->SceneRenderer;
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setSceneRenderWindow(vtkRenderWindow* renderWindow)
{
  if (renderWindow)
    {
    this->SceneRenderWindow = renderWindow;
    this->SceneRenderWindow->AddRenderer(this->SceneRenderer);

    this->VTKConnect->Connect(
      this->SceneRenderWindow->GetInteractor(),
      vtkCommand::MiddleButtonPressEvent, this, SLOT(onMiddleClick()));

    this->VTKConnect->Connect(
      this->SceneRenderWindow->GetInteractor(),
      vtkCommand::KeyPressEvent, this, SLOT(onKeyPress()));

    this->VTKConnect->Connect(
      this->SceneRenderWindow->GetInteractor(),
      vtkCommand::EndInteractionEvent, this, SLOT(onInteraction()));

    this->VTKConnect->Connect(
      this->SceneRenderer,
      vtkCommand::ResetCameraEvent, this, SLOT(onResetCamera()));
    }
  else
    {
    qDebug() << "ERROR: Invalid ( null ) render window";
    }
}

//-----------------------------------------------------------------------------
vtkRenderWindow* vpQtViewer3d::getSceneRenderWindow() const
{
  return this->SceneRenderWindow;
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setInteractorStyle(vtkInteractorStyle* style)
{
  if (!style)
    {
    qDebug() << "ERROR: Invalid ( null ) interactor style";
    }
  else
    {
    this->InteractorStyle = style;
    }
}

//-----------------------------------------------------------------------------
vtkInteractorStyle* vpQtViewer3d::getInteractorStyle() const
{
  return this->InteractorStyle;
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setPicker(vtkVgPicker* picker)
{
  if (!picker)
    {
    qDebug() << "ERROR: Invalid ( null ) picker";
    }

  this->Picker = picker;
}

//-----------------------------------------------------------------------------
vtkVgPicker* vpQtViewer3d::getPicker() const
{
  return this->Picker;
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setViewCoreInstance(vpViewCore* core)
{
  if (this->CoreInstance != core)
    {
    this->CoreInstance = core;
    }
}

//-----------------------------------------------------------------------------
vpViewCore* vpQtViewer3d::getViewCoreInstance()
{
  return this->CoreInstance;
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setContextDataSource(vpFileDataSource* source)
{
  if (source && this->ContextDataSource != source)
    {
    this->ContextDataSource = source;
    }
  else
    {
    qDebug() << "ERROR: Invalid ( null ) context source";
    }
}

//-----------------------------------------------------------------------------
vtkVgBaseImageSource* vpQtViewer3d::getContextSource() const
{
  return this->Internal->ContextSource;
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setProject(vpProject* project)
{
  if (project && project != this->Project)
    {
    this->Project = project;
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::addRepresentation(vtkVgRepresentationBase* representation)
{
  if (representation)
    {
    if (vtkVgGraphRepresentation::SafeDownCast(representation))
      {
      this->GraphRepresentation =
        vtkVgGraphRepresentation::SafeDownCast(representation);
      }

    this->Representations.push_back(representation);

    vtkPropCollection* props =
      representation->GetActiveRenderObjects();
    props->InitTraversal();
    while (vtkProp* prop = props->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }
  else
    {
    qDebug() << "ERROR: Invalid ( null ) representation";
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::update(const vtkVgTimeStamp& timestamp)
{
  if (this->ContextDataSource)
    {
    this->updateContext(timestamp);

    if (!this->SceneRenderer->HasViewProp(this->ContextActor))
      {
      this->SceneRenderer->AddViewProp(this->ContextActor);
      }
    }

  for (int i = 0; i < this->Representations.size(); ++i)
    {
    vtkVgRepresentationBase* representation = this->Representations[i];

    vtkPropCollection* expiredProps =
      representation->GetExpiredRenderObjects();
    expiredProps->InitTraversal();
    while (vtkProp* prop = expiredProps->GetNextProp())
      {
      this->SceneRenderer->RemoveViewProp(prop);
      }

    vtkPropCollection* newProps =
      representation->GetNewRenderObjects();
    newProps->InitTraversal();
    while (vtkProp* prop = newProps->GetNextProp())
      {
      this->SceneRenderer->AddViewProp(prop);
      }
    }

// Do nothing other than render.
  this->forceRender();
}


//-----------------------------------------------------------------------------
void vpQtViewer3d::forceRender()
{
  this->SceneRenderWindow->Render();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::reset()
{
  // For now just reset the camera.
  this->SceneRenderer->ResetCamera();

  this->forceRender();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::pick()
{
  vtkRenderWindowInteractor* interactor = this->SceneRenderWindow->GetInteractor();
  int x = interactor->GetEventPosition()[0];
  int y = interactor->GetEventPosition()[1];

  if (!this->Internal->Selector)
    {
    this->Internal->Selector = vtkSmartPointer<vtkHardwareSelector>::New();
    this->Internal->Selector->SetRenderer(this->SceneRenderer);
    this->Internal->Selector->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_POINTS);
    }

  this->Internal->Selector->SetArea(static_cast<unsigned int>(x),
                                    static_cast<unsigned int>(y),
                                    static_cast<unsigned int>(x),
                                    static_cast<unsigned int>(y));

  vtkSmartPointer<vtkSelection> selection;
  selection.TakeReference(this->Internal->Selector->Select());

  vtkVgPickData::PickData data = this->GraphRepresentation->Pick(selection);

  if (!data.PickedEntities.empty())
    {
    for (size_t i = 0; i < data.PickedEntities.size(); ++i)
      {
      std::cout << "Type: " << data.PickedEntities[i].PickedType << std::endl;
      std::cout << "Id: "   << data.PickedEntities[i].PickedId   << std::endl;
      }
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::getGraphEdgeColorModes(QStringList& list)
{
  for (int i = 0; i < vtkVgGraphRepresentation::CountEdgeColorModes; ++i)
    {
    list.push_back(qtString(vtkVgGraphRepresentation::GetGraphEdgeColorModeString(i)));
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::getGraphEdgeThicknessModes(QStringList& list)
{
  for (int i = 0; i < vtkVgGraphRepresentation::CountEdgeThicknessModes; ++i)
    {
    list.push_back(qtString(vtkVgGraphRepresentation::GetGraphEdgeThicknessModeString(i)));
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::onMiddleClick()
{
  this->pick();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::onKeyPress()
{
  switch (this->SceneRenderWindow->GetInteractor()->GetKeyCode())
    {
    case 'a':
    case 'A':
      this->resetToAOI();
      break;
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::onInteraction()
{
  this->updateContext(this->CoreInstance->getImageryTimeStamp());
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::onResetCamera()
{
  if (this->CoreInstance)
    {
    this->updateContext(this->CoreInstance->getImageryTimeStamp());
    }

  this->forceRender();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::resetToAOI()
{
  if (this->CoreInstance)
    {
    double extents[4];
    this->CoreInstance->getAOIExtents(extents);

    this->resetCamera();

    vtkVgRendererUtils::ZoomToExtents2D(this->SceneRenderer, extents, false);

    this->updateContext(this->CoreInstance->getImageryTimeStamp());

    this->forceRender();
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::resetCamera()
{
  this->SceneRenderer->GetActiveCamera()->SetViewUp(0.0, 1.0, 0.0);
  this->SceneRenderer->GetActiveCamera()->SetFocalPoint(0.0, 0.0, 0.0);
  this->SceneRenderer->GetActiveCamera()->SetPosition(0.0, 0.0, 1.0);
  this->SceneRenderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setGraphEdgeColorMode(int mode)
{
  if (this->GraphRepresentation)
    {
    this->GraphRepresentation->SetGraphEdgeColorMode(mode);
    this->forceRender();
    }
}

//-----------------------------------------------------------------------------
int vpQtViewer3d::getGraphEdgeColorMode()
{
  if (this->GraphRepresentation)
    {
    return this->GraphRepresentation->GetGraphEdgeColorMode();
    }
  else
    {
    return -1;
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setGraphEdgeThicknessMode(int mode)
{
  if (this->GraphRepresentation)
    {
    this->GraphRepresentation->SetGraphEdgeThicknessMode(mode);
    this->forceRender();
    }
}

//-----------------------------------------------------------------------------
int vpQtViewer3d::getGraphEdgeThicknessMode()
{
  return this->GraphRepresentation->GetGraphEdgeThicknessMode();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setGraphNodeHeightMode(int mode)
{
  if (this->GraphRepresentation)
    {
    vtkVgGraphModel* model = this->GraphRepresentation->GetGraphModel();

    model->SetCurrentZOffsetMode(mode);

    model->Update(model->GetCurrentTimeStamp());

    this->update(this->CoreInstance->getImageryTimeStamp());
    }
}

//-----------------------------------------------------------------------------
int vpQtViewer3d::getGraphNodeHeightMode()
{
  return this->GraphRepresentation->GetGraphModel()->GetCurrentZOffsetMode();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::getViewClippedWorldExtents(double points[][4],
                                              int numberOfPoints,
                                              double worldExtents[6])
{
  // In VTK view extents are -1, +1.
  const double e = std::numeric_limits<double>::epsilon();
  double viewExtents[2] = { -1.0, 1.0 };

  std::vector<double> intersectedWorldPoints;
  std::vector<double> intersectedViewPoints;

  for (int i = 0; i < numberOfPoints; ++i)
    {
    if (points[i][0] >= (viewExtents[0] - e) &&
        points[i][0] <= (viewExtents[1] + e) &&
        points[i][1] >= (viewExtents[0] - e) &&
        points[i][1] <= (viewExtents[1] + e))
      {
      intersectedViewPoints.push_back(points[i][0]);
      intersectedViewPoints.push_back(points[i][1]);
      intersectedViewPoints.push_back(points[i][2]);
      }
    }

  // Plane normals.
  double viewNormals[4][3] =
    {
      { viewExtents[1], 0.0, 0.0 },
      { 0.0, viewExtents[1], 0.0 },
      { viewExtents[0], 0.0, 0.0 },
      { 0.0, viewExtents[0], 0.0 }
    };

  // Points on the plane.
  double viewPoints[4][3] =
    {
      { viewExtents[1], viewExtents[0], 0.0 },
      { viewExtents[1], viewExtents[1], 0.0 },
      { viewExtents[0], viewExtents[1], 0.0 },
      { viewExtents[0], viewExtents[0], 0.0 }
    };

  for (int i = 0; i < numberOfPoints; ++i)
    {
    double point1[3] = { points[i][0], points[i][1], points[i][2] };
    double point2[3];

    if (i == numberOfPoints - 1)
      {
      point2[0] = points[0][0];
      point2[1] = points[0][1];
      point2[2] = points[0][2];
      }
    else
      {
      point2[0] = points[i + 1][0];
      point2[1] = points[i + 1][1];
      point2[2] = points[i + 1][2];
      }

    for (int j = 0; j < 4; ++j)
      {
      double intersectedPoint[3];

      bool intersected =
        vtkVgRendererUtils::GetLinePlaneIntersection(
          point1, point2, viewNormals[j], viewPoints[j], intersectedPoint);

      if (intersected &&
          (intersectedPoint[0] >= (viewExtents[0] - e)) &&
          (intersectedPoint[0] <= (viewExtents[1] + e)) &&
          (intersectedPoint[1] >= (viewExtents[0] - e)) &&
          (intersectedPoint[1] <= (viewExtents[1] + e)))
        {
        intersectedViewPoints.push_back(intersectedPoint[0]);
        intersectedViewPoints.push_back(intersectedPoint[1]);
        intersectedViewPoints.push_back(intersectedPoint[2]);
        }
      }
    }

  // Now check for points that are inside the polygon formed by joining points.
  this->Picker->SetActor(this->Internal->ContextPlaneActor);
  for (int i = 0; i < 4; ++i)
    {
    double displayPoint[4];

    vtkVgSpaceConversion::ViewToDisplay(this->SceneRenderer, viewPoints[i],
                                        displayPoint);

    int pickStatus =
      this->Picker->Pick(displayPoint[0], displayPoint[1], 0.0, this->SceneRenderer);

    if (pickStatus == vtkVgPickData::PickedImage)
      {
      double pickedPosition[3];
      this->Picker->GetPickPosition(pickedPosition);

      intersectedWorldPoints.push_back(pickedPosition[0]);
      intersectedWorldPoints.push_back(pickedPosition[1]);
      intersectedWorldPoints.push_back(pickedPosition[2]);
      }
    }

  worldExtents[0] = VTK_DOUBLE_MAX;
  worldExtents[1] = VTK_DOUBLE_MIN;
  worldExtents[2] = VTK_DOUBLE_MAX;
  worldExtents[3] = VTK_DOUBLE_MIN;
  worldExtents[4] = VTK_DOUBLE_MAX;
  worldExtents[5] = VTK_DOUBLE_MIN;

  for (size_t i = 0; i < intersectedViewPoints.size(); i += 3)
    {
    double viewPoint[3] =
      {
      intersectedViewPoints[i],
      intersectedViewPoints[i + 1],
      intersectedViewPoints[i + 2]
      };

    double worldPoint[4];

    vtkVgSpaceConversion::ViewToWorld(this->SceneRenderer, viewPoint,
                                      worldPoint);

    intersectedWorldPoints.push_back(worldPoint[0]);
    intersectedWorldPoints.push_back(worldPoint[1]);
    intersectedWorldPoints.push_back(worldPoint[2]);
    }

  if (intersectedWorldPoints.size())
    {
    vtkVgRendererUtils::CalculateBounds(intersectedWorldPoints, worldExtents);

    // Extents are 1 less than bounds.
    worldExtents[1] += -1;
    worldExtents[3] += -1;
    worldExtents[5] += -1;
    }
  else
    {
    worldExtents[0] = -1;
    worldExtents[1] = -1;
    worldExtents[2] = -1;
    worldExtents[3] = -1;
    worldExtents[4] = -1;
    worldExtents[5] = -1;
    }

//   \note: Debug messages.
//  qDebug() << worldExtents[0] << worldExtents[1] << worldExtents[2]
//           << worldExtents[3];
}

//-----------------------------------------------------------------------------
int vpQtViewer3d::calculateContextLevelOfDetail(double visibleExtents[6])
{
  double level = static_cast<double>(
                   this->Internal->ContextSource->GetNumberOfLevels());

  double viewableArea = (visibleExtents[1] - visibleExtents[0]) *
                        (visibleExtents[3] - visibleExtents[2]);

  double maxBounds[6];
  this->Internal->ContextPlaneActor->GetBounds(maxBounds);
  double maxViewableArea = (maxBounds[1] - maxBounds[0]) *
                           (maxBounds[3] - maxBounds[2]) ;

//  \note: Debug messages.
//  qDebug() << "viewableArea"  << viewableArea;
//  qDebug() << "maxViewableArea" << maxViewableArea;
  const double delayHighResolutionFactor = 1.2;

  double range[2] = { 1.0, level };
  level = pow(2,
              (level * viewableArea / maxViewableArea) *
              delayHighResolutionFactor *
              this->Internal->ContextSourceLODFactor);

  vtkMath::ClampValue(&level, range);

  //  \note: Debug messages.
//  qDebug() << "level" << level;

  return static_cast<int>(level);
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::setContextLevelOfDetailFactor(int factor)
{
  if (factor < 0)
    {
    this->Internal->ContextSourceLODFactor = static_cast<double>(abs(factor));
    }
  else if (factor > 0)
    {
    this->Internal->ContextSourceLODFactor = 1.0 / abs(factor);
    }
  else
    {
    this->Internal->ContextSourceLODFactor = 1.0;
    }

  if (!this->Internal->ContextSource)
    {
    return;
    }

  double extents[6] = {0.0};
  int readExtents[4] = {0};

  this->Internal->ContextSource->GetReadExtents(readExtents);

  extents[0] = static_cast<double>(readExtents[0]);
  extents[1] = static_cast<double>(readExtents[1]);
  extents[2] = static_cast<double>(readExtents[2]);
  extents[3] = static_cast<double>(readExtents[3]);

  this->Internal->ContextSource->SetLevel(
    this->calculateContextLevelOfDetail(extents));

  this->refreshContext();
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::updateContext(const vtkVgTimeStamp& timestamp)
{
  if (!this->ContextDataSource)
    {
    return;
    }

  std::string nextDataFile =
    this->ContextDataSource->getDataFile(timestamp.GetFrameNumber());

  if (!this->Internal->ContextSource)
    {
    this->Internal->ContextSource.TakeReference(
      vpImageSourceFactory::GetInstance()->Create(nextDataFile));
    this->Internal->ContextSource->SetOrigin(this->Project->OverviewOrigin.x(),
                                             this->Project->OverviewOrigin.y(),
                                             0.0);
    if (this->Project->OverviewSpacing >= 0.0)
      {
      this->Internal->ContextSource->SetSpacing(this->Project->OverviewSpacing,
                                                this->Project->OverviewSpacing,
                                                this->Project->OverviewSpacing);
      }
    }

  this->Internal->ContextSource->SetFileName(
    this->ContextDataSource->getDataFile(timestamp.GetFrameNumber()).c_str());

  double bounds[6];
  this->Internal->ContextPlaneActor->GetBounds(bounds);

  double pt1[3] = {bounds[0], bounds[2], bounds[5]}; // min, min
  double pt2[3] = {bounds[0], bounds[3], bounds[5]}; // min, max
  double pt3[3] = {bounds[1], bounds[3], bounds[5]}; // max, max
  double pt4[3] = {bounds[1], bounds[2], bounds[5]}; // max, min

  double outPoints[4][4];

  // Make sure camera exists, otherwise view transformation will fail
  this->SceneRenderer->GetActiveCamera();

  vtkVgSpaceConversion::WorldToView(this->SceneRenderer, pt1, &outPoints[0][0]);
  vtkVgSpaceConversion::WorldToView(this->SceneRenderer, pt2, &outPoints[1][0]);
  vtkVgSpaceConversion::WorldToView(this->SceneRenderer, pt3, &outPoints[2][0]);
  vtkVgSpaceConversion::WorldToView(this->SceneRenderer, pt4, &outPoints[3][0]);

  double readExtents[6];

  this->getViewClippedWorldExtents(outPoints, 4, readExtents);

  // Ignoring z extents.
  readExtents[4] = 0.0;
  readExtents[5] = 0.0;

  if (!this->Internal->Initialized)
    {
    this->ContextActor->SetInputData(this->Internal->ContextData);

    this->Internal->ContextSource->SetLevel(3);
    this->Internal->ContextSource->SetReadExtents(-1, -1, -1, -1);
    }
  else
    {
    for (int i = 0; i < 4; ++i)
      {
      if (static_cast<int>(readExtents[i]) == -1)
        {
        continue;
        }

      if (i < 2)
        {
        readExtents[i] -= this->Project->OverviewOrigin.x();
        }
      else
        {
        readExtents[i] -= this->Project->OverviewOrigin.y();
        }
      }

    //   \note: Debug messages.
    //  qDebug() << readExtents[0] << readExtents[1] << readExtents[2]
    //           << readExtents[3];

    this->Internal->ContextSource->SetReadExtents(
      static_cast<int>(readExtents[0]),
      static_cast<int>(readExtents[1]),
      static_cast<int>(readExtents[2]),
      static_cast<int>(readExtents[3]));

    this->Internal->ContextSource->SetLevel(
      this->calculateContextLevelOfDetail(readExtents));
    }

  this->refreshContext();

  if (!this->Internal->Initialized)
    {
    double contextBounds[6];
    this->ContextActor->GetBounds(contextBounds);

    this->Internal->ContextPlane->SetOrigin(
      contextBounds[0], contextBounds[2], contextBounds[5]);

    this->Internal->ContextPlane->SetPoint1(
      contextBounds[1], contextBounds[2], contextBounds[5]);

    this->Internal->ContextPlane->SetPoint2(
      contextBounds[0], contextBounds[3], contextBounds[5]);

    vtkSmartPointer<vtkPolyDataMapper> mapper(
      vtkSmartPointer<vtkPolyDataMapper>::New());
    mapper->SetInputData(this->Internal->ContextPlane->GetOutput());

    this->Internal->ContextPlaneActor->SetMapper(mapper);
    this->Internal->ContextPlaneActor->GetProperty()->SetOpacity(0.0);

    this->SceneRenderer->AddActor(this->Internal->ContextPlaneActor);

    // For some reason this->Picker->SetImageActor(this->ContextActor) is not working.
    vtkSmartPointer<vtkImageData> imageData(
      vtkSmartPointer<vtkImageData>::New());
    imageData->ShallowCopy(this->Internal->ContextData);

    vtkSmartPointer<vtkImageActor> imageActor(
      vtkSmartPointer<vtkImageActor>::New());
    imageActor->SetInputData(imageData);

    this->Picker->SetImageActor(imageActor);

    this->Internal->Initialized = true;

    emit this->contextCreated();
    }
}

//-----------------------------------------------------------------------------
void vpQtViewer3d::refreshContext()
{
  this->Internal->ContextSource->Update();

  this->Internal->ContextData->ShallowCopy(
    this->Internal->ContextSource->GetOutput());

  this->ContextActor->Update();
}
