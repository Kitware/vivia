/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvClipVideoRepresentation.h"

// VG includes.
#include "vtkVgAdapt.h"
#include "vtkVgEvent.h"
#include "vtkVgEventModel.h"
#include "vtkVgEventRegionRepresentation.h"
#include "vtkVgEventRepresentationBase.h"
#include "vtkVgTrackRepresentationBase.h"
#include "vtkVgVideoMetadata.h"
#include "vtkVgVideoModel0.h"
#include "vtkVgVideoProviderBase.h"

// VTK includes.
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>

vtkStandardNewMacro(vvClipVideoRepresentation);

//-----------------------------------------------------------------------------
vvClipVideoRepresentation::vvClipVideoRepresentation() : QObject(),
  vtkVgVideoRepresentation0()
{
  this->AutoCenter    = 0;

  this->RegionsMaxWidth  = 0.0;
  this->RegionsMaxHeight = 0.0;

  this->VGEventCached = 0;
}

//-----------------------------------------------------------------------------
vvClipVideoRepresentation::~vvClipVideoRepresentation()
{
  // Do nothing.
}

//-----------------------------------------------------------------------------
void vvClipVideoRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vvClipVideoRepresentation::Update()
{
  if (this->VideoModel->GetUpdateDataRequestTime() > this->UpdateTime.GetMTime() ||
      this->GetUpdateRequest())
    {
    const vtkVgVideoFrameData* frameData = this->VideoModel->GetFrameData();

    if (frameData)
      {
      // \NOTE: For this see notes above in the constructor.
      this->VideoActor->SetUseBounds(1);

      this->VideoActor->SetInputData(frameData->VideoImage);

      // Calculate the render matrix.
      this->RenderMatrix->Identity();

      if (frameData->VideoMatrix)
        {
        this->RenderMatrix->DeepCopy(frameData->VideoMatrix);
        }

      // HACK - This is needed to make picking work correctly.
      this->MakeLinear(this->RenderMatrix);

      if (this->EventRepresentation)
        {
        if (this->AutoCenter)
          {
          this->AutoCenterUpate(frameData);
          } // if (this->AutoCenter)
        this->EventRepresentation->Update();
        }

      if (this->TrackRepresentation)
        {
        vtkVgVideoProviderBase* videoSource = this->VideoModel->GetVideoSource();

        if (videoSource)
          {
          vtkVgVideoMetadata metadata;

          if (videoSource->GetCurrentMetadata(&metadata) == VTK_OK)
            {
            double ib[6];
            this->VideoActor->GetBounds(ib);

            vtkSmartPointer<vtkMatrix4x4> hi(vtkSmartPointer<vtkMatrix4x4>::New());
            vtkSmartPointer<vtkMatrix4x4> fy(vtkSmartPointer<vtkMatrix4x4>::New());
            vtkSmartPointer<vtkMatrix4x4> xf(vtkSmartPointer<vtkMatrix4x4>::New());

            hi->DeepCopy(metadata.Homography);
            hi->Invert();
            fy->Identity();
            fy->SetElement(1, 1, -1);
            fy->SetElement(1, 3, ib[3] - ib[2]);
            vtkMatrix4x4::Multiply4x4(fy, hi, xf);

            // HACK - This is needed to make picking work correctly.
            this->MakeLinear(xf);

            this->TrackRepresentation->SetRepresentationMatrix(xf.GetPointer());
            } // if (videoFrame)
          } // if (videoSource)
        this->TrackRepresentation->Update();
        } // if (this->TrackRepresentation)


      this->UpdateTime.Modified();
      } // if (frameData)
    }
}

//-----------------------------------------------------------------------------
void vvClipVideoRepresentation::AutoCenterUpate(const vtkVgVideoFrameData* frameData)
{
  // If this is the first time.
  if (!this->VGEventCached)
    {
    vtkVgEventModel* eventModel = this->EventRepresentation->GetEventModel();
    if (eventModel)
      {
      eventModel->InitEventTraversal();

      // \NOTE: There should be only one event (which is true for now).
      this->VGEventCached = eventModel->GetNextEvent().GetEvent();
      }

    // Now find the biggest event region.
    if (this->VGEventCached)
      {
      this->VGEventCached->InitRegionTraversal();

      vtkVgTimeStamp timestamp;
      vtkIdType numIds;
      vtkIdType* ids;
      vtkPoints* points = this->VGEventCached->GetRegionPoints();

      // first pass - figure out the biggest region in the event
      this->VGEventCached->InitRegionTraversal();
      while (this->VGEventCached->GetNextRegion(timestamp, numIds, ids))
        {
        double ul[3], lr[3];
        points->GetPoint(ids[0], ul);
        points->GetPoint(ids[2], lr);

        this->RegionsMaxWidth  = std::max(this->RegionsMaxWidth, lr[0] - ul[0]);
        this->RegionsMaxHeight = std::max(this->RegionsMaxHeight, ul[1] - lr[1]);
        }
      }
    } // if (!this->VGEventCached)

  if (this->VGEventCached)
    {
    double center[3];
    this->VGEventCached->GetRegionCenter(frameData->TimeStamp,
                                         center, true);

    emit this->areaOfInterest(center, this->RegionsMaxWidth, this->RegionsMaxHeight);
    }
}


//-----------------------------------------------------------------------------
void vvClipVideoRepresentation::HandleAnimationCueTickEvent()
{
  this->SetEventVisible(1);
}



