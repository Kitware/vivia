/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVQTrackingClip.h"

#include "vtkVgEvent.h"
#include "vtkVgVideoNode.h"
#include "vtkVgVideoProviderBase.h"
#include "vtkVgVideoFrameData.h"

#include <vtkDoubleArray.h>
#include <vtkExtractVOI.h>
#include <vtkFieldData.h>
#include <vtkImageAppend.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>

#include <algorithm>

vtkStandardNewMacro(vtkVQTrackingClip);

//-----------------------------------------------------------------------------
vtkVQTrackingClip::vtkVQTrackingClip()
  : Rank(-1)
{
  this->Dimensions[0] = 100;
  this->Dimensions[1] = 100;
  this->OutputImageData = vtkSmartPointer<vtkImageData>::New();
}

//-----------------------------------------------------------------------------
vtkVQTrackingClip::~vtkVQTrackingClip()
{
}

//-----------------------------------------------------------------------------
void vtkVQTrackingClip::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkVQTrackingClip::SetVideo(vtkVgVideoProviderBase* vs)
{
  this->Video = vs;
}

//-----------------------------------------------------------------------------
void vtkVQTrackingClip::SetEvent(vtkVgEvent* event)
{
  this->Event = event;
}

//-----------------------------------------------------------------------------
void vtkVQTrackingClip::SetVideoNode(vtkVgVideoNode* node)
{
  this->VideoNode = node;
}

//-----------------------------------------------------------------------------
vtkVgVideoNode* vtkVQTrackingClip::GetVideoNode()
{
  return this->VideoNode;
}

//-----------------------------------------------------------------------------
vtkImageData* vtkVQTrackingClip::GetOutputImageData()
{
  if (!this->Video)
    {
    vtkErrorMacro("Source video not set.");
    return 0;
    }

  if (!this->Event)
    {
    vtkErrorMacro("Event to track not set.");
    return 0;
    }

  if (this->UpdateTime < this->GetMTime())
    {
    this->Update();
    }

  return this->OutputImageData;
}

//-----------------------------------------------------------------------------
double vtkVQTrackingClip::GetDuration()
{
  return this->Video->GetTimeRange()[1] - this->Video->GetTimeRange()[0];
}

//-----------------------------------------------------------------------------
vtkIdType vtkVQTrackingClip::GetEventId()
{
  if (this->Event)
    {
    return this->Event->GetId();
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vtkVQTrackingClip::Update()
{
  this->UpdateTime.Modified();

  vtkVgTimeStamp timestamp;
  vtkIdType numIds;
  vtkIdType* ids;

  vtkPoints* points = this->Event->GetRegionPoints();

  double maxDim = 0.0;

  // first pass - figure out the biggest region in the event
  this->Event->InitRegionTraversal();
  while (this->Event->GetNextRegion(timestamp, numIds, ids))
    {
    double ul[3], lr[3];
    points->GetPoint(ids[0], ul);
    points->GetPoint(ids[2], lr);

    double longestSide = std::max(lr[0] - ul[0], ul[1] - lr[1]);
    maxDim = std::max(maxDim, longestSide);
    }

  int sampleRate = 1;
  if (maxDim > Dimensions[0] || maxDim > Dimensions[1])
    {
    std::cout << "Event region bounds exceed tracking clip dimensions - "
              << "downsampling will occur.\n";

    int minDim = std::min(Dimensions[0], Dimensions[1]);
    int div = maxDim / minDim;
    while (++sampleRate < 4 && (div >>= 1));
    }

  int regionDim[] = { this->Dimensions[0]* sampleRate,
                      this->Dimensions[1]* sampleRate
                    };

  double videoTimeRange[2];
  this->Video->GetTimeRange(videoTimeRange);

  vtkVgTimeStamp eventStart = this->Event->GetStartFrame();
  vtkVgTimeStamp eventEnd = this->Event->GetEndFrame();

  // sanity checks
  if (eventStart.GetTime() < videoTimeRange[0])
    {
    vtkErrorMacro("Event starts outside of video source time range.");
    eventStart.SetTime(videoTimeRange[0]);
    eventStart.SetFrameNumber(this->Video->GetFrameRange()[0]);
    }
  if (eventEnd.GetTime() > videoTimeRange[1])
    {
    vtkErrorMacro("Event ends outside of video source time range.");
    eventEnd.SetTime(videoTimeRange[1]);
    eventEnd.SetFrameNumber(this->Video->GetFrameRange()[1]);
    }

  this->Video->SetLooping(0);

  // seek to the start of the event
  vtkVgVideoFrameData frameData;
  this->Video->GetFrame(&frameData, eventStart.GetTime());

  int dim[3];
  frameData.VideoImage->GetDimensions(dim);
  if (regionDim[0] > dim[0] || regionDim[1] > dim[1])
    {
    vtkErrorMacro("Tracking clip extents exceed input video dimensions.")
    }

  vtkImageAppend* IAInput = vtkImageAppend::New();
  IAInput->SetNumberOfThreads(1);
  IAInput->SetAppendAxis(2);

  vtkImageAppend* IA = vtkImageAppend::New();
  IA->SetNumberOfThreads(1);
  IA->SetAppendAxis(2);

  vtkDoubleArray* timeStampData = vtkDoubleArray::New();
  timeStampData->SetName("TimeStampData");

  // loop over the video
  do
    {
    // past end of event?
    if (eventEnd < frameData.TimeStamp)
      {
      break;
      }

    double center[2];
    bool interpolated = false;
    bool found = this->Event->GetRegionCenter(frameData.TimeStamp,
                                              center, interpolated);

    // this should never happen, unless no region exists!
    if (!found)
      {
      std::cout << "Missing region - interpolating tracking position.\n";
      center[0] = dim[0] / 2;
      center[1] = dim[1] / 2;
      }

    int ll[] =
      {
      static_cast<int>(center[0] - regionDim[0] / 2),
      static_cast<int>(center[1] - regionDim[1] / 2)
      };

    int ur[] =
      {
      ll[0] + regionDim[0] - 1,
      ll[1] + regionDim[1] - 1
      };

    // keep the clipping region within the bounds of the video
    if (ll[0] < 0)
      {
      ur[0] += -ll[0];
      ll[0] = 0;
      }
    if (ll[1] < 0)
      {
      ur[1] += -ll[1];
      ll[1] = 0;
      }
    if (ur[0] > dim[0] - 1)
      {
      ll[0] -= ur[0] - (dim[0] - 1);
      ur[0] = dim[0] - 1;
      }
    if (ur[1] > dim[1] - 1)
      {
      ll[1] -= ur[1] - (dim[1] - 1);
      ur[1] = dim[1] - 1;
      }

    int wholeExtent[6] =
      {
      ll[0], ur[0],
      ll[1], ur[1],
      0,     0
      };

    // crop to the area of interest
    vtkExtractVOI* IC = vtkExtractVOI::New();
    IC->SetInputData(frameData.VideoImage);
    IC->SetVOI(wholeExtent);
    IC->SetSampleRate(sampleRate, sampleRate, 1);

    // make sure cropped images are aligned
    vtkImageChangeInformation* ICI = vtkImageChangeInformation::New();
    ICI->SetInputConnection(IC->GetOutputPort());
    ICI->SetOutputExtentStart(0, 0, 0);
    ICI->SetOutputSpacing(1.0, 1.0, 1.0);
    IC->FastDelete();

    IAInput->AddInputConnection(ICI->GetOutputPort());
    ICI->FastDelete();

    // process input frames in small batches to achieve fast execution
    // while limiting the number of input frames in memory at any given time
    if (IAInput->GetNumberOfInputConnections(0) == 8)
      {
      IAInput->Update();
      vtkImageData* copy = vtkImageData::New();
      copy->DeepCopy(IAInput->GetOutput());
      IAInput->Delete();
      IAInput = vtkImageAppend::New();
      IAInput->SetNumberOfThreads(1);
      IAInput->SetAppendAxis(2);
      IA->AddInputData(copy);
      copy->FastDelete();
      }

    // embed the timestamp
    timeStampData->InsertNextValue(frameData.TimeStamp.GetTime());

    // prevent the GetNextFrame call from de-registering our data
    frameData.VideoImage = vtkSmartPointer<vtkImageData>::New();
    }
  while (this->Video->GetNextFrame(&frameData) == VTK_OK);

  // add any remaining input frames
  if (IAInput->GetNumberOfInputs() > 0)
    {
    IA->AddInputConnection(IAInput->GetOutputPort());
    }

  IA->Update();
  this->OutputImageData->DeepCopy(IA->GetOutput());
  this->OutputImageData->GetFieldData()->AddArray(timeStampData);
  timeStampData->FastDelete();
  IAInput->Delete();
  IA->Delete();
}
