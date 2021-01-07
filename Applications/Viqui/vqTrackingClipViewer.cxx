// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vqTrackingClipViewer.h"

#include "vqCore.h"
#include "vqTrackingClipBuilder.h"
#include "vqUtil.h"
#include "vtkVQTrackingClip.h"

#include <vtkVgVideoNode.h>

#include <vgFileDialog.h>

#include <QVTKWidget.h>

#include <vtkCamera.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkPNMWriter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkWindowToImageFilter.h>

#include <QMenu>
#include <QSettings>
#include <QThread>
#include <QTime>
#include <QVBoxLayout>

#include <algorithm>
#include <sstream>
#include <vector>

namespace
{

//-----------------------------------------------------------------------------
enum
{
  MaxClips    = 20,
  ClipsPerRow = 5,
  Padding     = 5,
  VideoSize   = 100,
  UpdateInterval = 16  // ~60Hz
};

struct CompareByRank
{
  bool operator()(const vqTrackingClipBuilder::ClipElement& c1,
                  const vqTrackingClipBuilder::ClipElement& c2)
    {
    return c1.second->GetRank() < c2.second->GetRank();
    }
};

} // end anonymous namespace

//-----------------------------------------------------------------------------
struct vqTrackingClipViewer::Cell
{
  vtkSmartPointer<vtkRenderer>   Renderer;
  vtkSmartPointer<vtkImageActor> Image;
  vtkSmartPointer<vtkTextActor>  Label;
  bool RendererInitialized;
  bool Used;
  int  Id;

  QTime StartTime;
  QTime LastUpdate;

  Cell() : RendererInitialized(false), Used(false), Id(-1)
    {
    }
};

//-----------------------------------------------------------------------------
class vtkInteractorCallback : public vtkCommand
{
  vqTrackingClipViewer* TrackingClipViewer;
  QTime TimeBetweenRenders;
  QTime LastClick;
  int LastClickPos[2];

  vtkWindowToImageFilter* WindowToImageFilter;
  vtkPNMWriter* PNMWriter;
  QString ImageOutputDirectory;
  vtkIdType ImageCounter;
  bool WriteRenderedImages;

public:
  static vtkInteractorCallback* New()
    {
    return new vtkInteractorCallback;
    }

  vtkInteractorCallback()
    {
    this->WindowToImageFilter = vtkWindowToImageFilter::New();
    this->PNMWriter = vtkPNMWriter::New();
    this->ImageCounter = 0;
    this->WriteRenderedImages = false;
    }
  ~vtkInteractorCallback()
    {
    this->WindowToImageFilter->Delete();
    this->PNMWriter->Delete();
    }

  void SetTrackingClipViewer(vqTrackingClipViewer* tcv)
    {
    this->TrackingClipViewer = tcv;
    this->WindowToImageFilter->SetInput(tcv->GetWidget()->GetRenderWindow());
    this->PNMWriter->SetInputConnection(this->WindowToImageFilter->GetOutputPort());
    }

  virtual void Execute(vtkObject* vtkNotUsed(caller), unsigned long eventId,
                       void* vtkNotUsed(callData))
    {
    switch (eventId)
      {
      case vtkCommand::TimerEvent:
        this->UpdateEvent();
        break;

      case vtkCommand::LeftButtonPressEvent:
        this->LeftClickEvent();
        break;

      case vtkCommand::MiddleButtonPressEvent:
        this->MiddleClickEvent();
        break;

      case vtkCommand::RightButtonReleaseEvent:
        this->RightClickEvent();
        break;

      case vtkCommand::KeyPressEvent:
        vtkRenderWindowInteractor* rwi =
          this->TrackingClipViewer->GetWidget()->GetInteractor();

        // toggle frame skip mode
        if (rwi->GetKeyCode() == 's')
          {
          bool prev = this->TrackingClipViewer->GetAllowFrameSkip();
          this->TrackingClipViewer->SetAllowFrameSkip(!prev);

          if (this->TrackingClipViewer->GetAllowFrameSkip())
            {
            std::cout << "Frame skipping enabled.\n";
            }
          else
            {
            std::cout << "Frame skipping disabled.\n";
            }
          }
        else if (rwi->GetKeyCode() == 'n')
          {
          this->TrackingClipViewer->RequestNextClips();
          }
        break;
      }
    }

private:
  void UpdateEvent()
    {
    // For some reason, setting the update timer interval to 33ms causes VTK/Qt
    // to only update the timer every 50-60ms. So we set the timer interval to
    // 16 and throw out half the updates, as a hack/workaround.
    if (!this->TimeBetweenRenders.isNull() &&
        this->TimeBetweenRenders.elapsed() < 33)
      {
      return;
      }

    // update clips
    bool clipChanged = false;
    size_t numCells = this->TrackingClipViewer->GetCellCount();
    for (size_t i = 0; i < numCells; ++i)
      {
      vqTrackingClipViewer::Cell& cell = this->TrackingClipViewer->GetCell(i);
      vtkImageActor* IA = cell.Image;
      if (!IA)
        {
        continue;
        }

      int fd;
      vtkDoubleArray* timeStamps =
        vtkDoubleArray::SafeDownCast(IA->GetInput()->GetFieldData()
                                     ->GetArray("TimeStampData", fd));
      if (!timeStamps)
        {
        std::cout << "Clip is missing timestamp data.\n";
        continue;
        }

      int slice = IA->GetZSlice();
      int maxSlice = IA->GetWholeZMax();
      // set prevSlice to -1 if ImageActor display extent is not valid
      int prevSlice = IA->GetDisplayExtent()[5] < 0 ? -1 : slice;

      // compute the frame to show for this update, if more than 1 frame
      if (maxSlice > 0 && this->TrackingClipViewer->GetAllowFrameSkip())
        {
        int et = cell.StartTime.elapsed();
        double clipStart = timeStamps->GetValue(0);

        // most of the time, showing the same frame or the very next frame
        bool search = true;
        if (slice < maxSlice - 1)
          {
          if (et < 1.0e-3 * (timeStamps->GetValue(slice + 1) - clipStart))
            {
            continue; // still showing same frame
            }
          if (et < 1.0e-3 * (timeStamps->GetValue(slice + 2) - clipStart))
            {
            ++slice;  // next frame
            search = false;
            }
          }

        // search for the last frame with an elapsed time less than our start time
        if (search)
          {
          int min = slice;
          int max = maxSlice;
          while (min <= max)
            {
            int mid = min + (max - min) / 2;
            double t = (timeStamps->GetValue(mid) - clipStart) * 1.0e-3;
            if (et > t)
              {
              min = mid + 1;
              }
            else
              {
              max = mid - 1;
              }
            }
          slice = min - 1;
          if (slice < 0)
            {
            slice = 0; // this shouldn't happen, unless there is something wrong
            }
          }

        // handle looping when reaching the end of the clip
        if (slice == maxSlice)
          {
          double maxTime = timeStamps->GetValue(maxSlice);
          if (et > (2 * maxTime -
                    timeStamps->GetValue(maxSlice - 1) - clipStart) * 1.0e-3)
            {
            slice = 0;
            cell.StartTime.restart();
            }
          }
        }
      else if (maxSlice > 0) // not skipping frames and more than 1 frame
        {
        // compute the amount of time to show this frame
        int tts;
        if (slice == maxSlice)
          {
          tts = 1.0e-3 * (timeStamps->GetValue(slice) -
                          timeStamps->GetValue(slice - 1));
          }
        else
          {
          tts = 1.0e-3 * (timeStamps->GetValue(slice + 1) -
                          timeStamps->GetValue(slice));
          }

        // has this frame been displayed long enough?
        int dt = cell.LastUpdate.elapsed();
        if (dt >= tts - UpdateInterval / 2)
          {
          slice += 1;
          if (slice > maxSlice)
            {
            slice = 0;
            }
          }
        //std::cout << "dt: " << dt << " tts: " << tts
        //          << " skew: " << dt - tts << " ms\n";
        }

      // update slice
      if (slice != prevSlice)
        {
        IA->Update();
        int* e = IA->GetInput()->GetExtent();
        IA->SetDisplayExtent(e[0], e[1], e[2], e[3], slice, slice);

        // adjust the clipping range - this assumes an input z-spacing of one
        // and origin at 0,0,0
        vtkCamera* camera = cell.Renderer->GetActiveCamera();
        int zdist = camera->GetPosition()[2] - slice;
        cell.Renderer->GetActiveCamera()->SetClippingRange(zdist - 1, zdist + 1);
        cell.LastUpdate.restart();
        clipChanged = true;

        int framesSkipped = slice < prevSlice ? slice + (maxSlice - prevSlice)
                            : slice - (prevSlice + 1);
        if (framesSkipped)
          {
          std::cout << "Skipped " << framesSkipped << " frame(s).\n";
          }
        }
      }

    // render
    if (clipChanged)
      {
      this->TrackingClipViewer->GetWidget()->GetRenderWindow()->Render();
      this->TimeBetweenRenders.restart();

      if (this->WriteRenderedImages)
        {
        this->WindowToImageFilter->Modified();
        const QChar zero('0');
        const QString outputFileName =
          this->ImageOutputDirectory +
          QString("/TrackingClipImage%1.pnm")
            .arg(this->ImageCounter++, 6, 10, zero);
        this->PNMWriter->SetFileName(qPrintable(outputFileName));
        this->PNMWriter->Write();
        }
      }
    }

  void LeftClickEvent()
    {
    int x, y;
    this->TrackingClipViewer->GetWidget()->GetInteractor()->GetEventPosition(x, y);

    // activate the clip if this is a double-click
    bool activate = false;
    if (this->LastClick.isNull())
      {
      this->LastClick.start();
      }
    else if (this->LastClick.restart() < 400)
      {
      if (abs(x - this->LastClickPos[0]) < 5 &&
          abs(y - this->LastClickPos[1]) < 5)
        {
        activate = true;
        }
      }

    this->LastClickPos[0] = x;
    this->LastClickPos[1] = y;
    this->TrackingClipViewer->SelectClipAt(x, y, activate);
    }

  void MiddleClickEvent()
    {
    int x, y;
    this->TrackingClipViewer->GetWidget()->GetInteractor()->GetEventPosition(x, y);
    this->TrackingClipViewer->RemoveClipAt(x, y);
    }

  void RightClickEvent()
    {
    int x, y;
    QVTKWidget* widget = this->TrackingClipViewer->GetWidget();
    widget->GetInteractor()->GetEventPosition(x, y);

    vtkVQTrackingClip* clip = this->TrackingClipViewer->SelectClipAt(x, y);
    if (!clip)
      {
      return;
      }

    vtkVgVideoNode* node = clip->GetVideoNode();

    // go back to Qt widget coords
    y = widget->GetInteractor()->GetSize()[1] - y - 1;

    QMenu menu(widget);

    QAction* good = menu.addAction(
                      vqUtil::uiIqrClassificationString(vvIqr::PositiveExample));
    good->setCheckable(true);
    good->setChecked(node->GetUserScore() == vvIqr::PositiveExample);

    QAction* bad = menu.addAction(
                     vqUtil::uiIqrClassificationString(vvIqr::NegativeExample));
    bad->setCheckable(true);
    bad->setChecked(node->GetUserScore() == vvIqr::NegativeExample);

    QAction* none = menu.addAction(
                      vqUtil::uiIqrClassificationString(vvIqr::UnclassifiedExample));
    none->setCheckable(true);
    none->setChecked(node->GetUserScore() == vvIqr::UnclassifiedExample);

    menu.addSeparator();

    QAction* write = menu.addAction("&Write Rendered Images");
    write->setCheckable(true);
    write->setChecked(this->WriteRenderedImages);

    QAction* picked = menu.exec(widget->mapToGlobal(QPoint(x, y)));

    if (picked == write)
      {
      // If currently off, turn it on once we have a directory from the user
      if (!this->WriteRenderedImages)
        {
        this->ImageOutputDirectory =
          vgFileDialog::getExistingDirectory(
            this->TrackingClipViewer->GetWidget()->window(),
            "Image Output Directory");
        if (!this->ImageOutputDirectory.isEmpty())
          {
          this->WriteRenderedImages = true;
          }
        }
      else  // Was writing, turn writing off
        {
        this->WriteRenderedImages = false;
        }
      }
    else // (picked == good || picked == bad || picked == none)
      {
      vvIqr::Classification c;
      if (picked == good)
        {
        c = vvIqr::PositiveExample;
        }
      else if (picked == bad)
        {
        c = vvIqr::NegativeExample;
        }
      else
        {
        c = vvIqr::UnclassifiedExample;
        }
      node->SetUserScore(c);
      emit this->TrackingClipViewer->ClipRatingChanged(node->GetInstanceId(), c);
      }
    }
};

//-----------------------------------------------------------------------------
class vqTrackingClipViewer::vqInternal
{
public:
  std::vector<Cell>        Cells;
  std::vector<ClipElement> Clips;

  vqTrackingClipBuilder ClipBuilder;
};

//-----------------------------------------------------------------------------
vqTrackingClipViewer::vqTrackingClipViewer(QWidget* parent)
  : QDialog(parent), Widget(0), HasBeenShown(false), AllowFrameSkip(true),
    NumRows(0), NumColumns(0), VideoScale(1.0), CurrentId(0)
{
  this->Widget = new QVTKWidget(this);
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(this->Widget);
  layout->setSpacing(0);
  layout->setMargin(0);
  this->setLayout(layout);

  this->Internal = new vqInternal;
  this->InteractorCallback = vtkSmartPointer<vtkInteractorCallback>::New();
  this->InteractorCallback->SetTrackingClipViewer(this);
}

//-----------------------------------------------------------------------------
vqTrackingClipViewer::~vqTrackingClipViewer()
{
  delete this->Internal;
  delete this->Widget;
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::showEvent(QShowEvent*)
{
  if (this->Internal->Clips.empty())
    {
    std::cerr << "No clips added.\n";
    return;
    }

  if (this->HasBeenShown)
    {
    this->Widget->GetInteractor()->AddObserver(vtkCommand::TimerEvent,
                                               this->InteractorCallback);
    return;
    }

  // disable interaction since the default behavior is not very useful
  this->Widget->GetInteractor()->SetInteractorStyle(0);

  this->Widget->GetInteractor()->AddObserver(vtkCommand::KeyPressEvent,
                                             this->InteractorCallback);
  this->Widget->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent,
                                             this->InteractorCallback);
  this->Widget->GetInteractor()->AddObserver(vtkCommand::MiddleButtonPressEvent,
                                             this->InteractorCallback);
  this->Widget->GetInteractor()->AddObserver(vtkCommand::RightButtonReleaseEvent,
                                             this->InteractorCallback);
  this->Widget->GetInteractor()->AddObserver(vtkCommand::TimerEvent,
                                             this->InteractorCallback);

  // restore position
  QSettings settings;
  settings.beginGroup("Clip Viewer Window");
  this->move(settings.value("position").toPoint());

  // start the update timer
  this->Widget->GetInteractor()->CreateRepeatingTimer(UpdateInterval);

  this->UpdateGridDimensions();
  this->UpdateIdealSize();

  this->resize(this->GetMinWindowSize());

  // Add a single renderer that covers the entire window - otherwise, the
  // padding regions may not be "cleared" since they are outside the bounds
  // of the clip renderers.
  vtkRenderer* ren = vtkRenderer::New();
  ren->SetLayer(0);
  this->Widget->GetRenderWindow()->AddRenderer(ren);
  ren->FastDelete();

  // initialize and lay out clip renderers
  for (size_t i = 0, end = this->GetCellCount(); i < end; ++i)
    {
    if (!this->Internal->Cells[i].RendererInitialized)
      {
      this->InitializeRenderer(this->Internal->Cells[i]);
      }
    }
  this->LayoutRenderers();

  connect(&this->Internal->ClipBuilder, SIGNAL(ClipAvailable(vtkImageData*, int)),
          this, SLOT(ClipAvailable(vtkImageData*, int)));

  // build the clip data in a background thread
  this->Internal->ClipBuilder.BuildClips(this->Internal->Clips.begin(),
                                         this->Internal->Clips.end());

  this->Widget->GetRenderWindow()->SetNumberOfLayers(2);
  this->Widget->GetRenderWindow()->Render();
  this->Widget->show();
  this->HasBeenShown = true;
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::hideEvent(QHideEvent*)
{
  // stop updating the clips
  this->Widget->GetInteractor()->RemoveObserver(this->InteractorCallback);
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.beginGroup("Clip Viewer Window");
  settings.setValue("position", this->pos());
  this->QDialog::closeEvent(event);
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::resizeEvent(QResizeEvent*)
{
  if (!this->HasBeenShown)
    {
    return;
    }
  double width = this->width();
  double height = this->height();

  this->VideoScale = std::min(width / this->IdealSize.width(),
                              height / this->IdealSize.height());
  this->LayoutRenderers();
}

//-----------------------------------------------------------------------------
size_t vqTrackingClipViewer::GetClipCount()
{
  return this->Internal->Clips.size();
}

//-----------------------------------------------------------------------------
size_t vqTrackingClipViewer::GetCellCount()
{
  return this->Internal->Cells.size();
}

//-----------------------------------------------------------------------------
vqTrackingClipViewer::Cell& vqTrackingClipViewer::GetCell(size_t index)
{
  return this->Internal->Cells[index];
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::InsertClips(const ClipVector& clips,
                                       bool allowDuplicates)
{
  // sort input by rank
  ClipVector clipsCopy(clips);
  std::sort(clipsCopy.begin(), clipsCopy.end(), CompareByRank());

  // crop to max size
  if (clipsCopy.size() > MaxClips)
    {
    clipsCopy.resize(MaxClips);
    }
  size_t inputSize = clipsCopy.size();

  if (!allowDuplicates && this->Internal->Clips.size() != 0)
    {
    // add new clips to the front of the vector
    this->Internal->Clips.insert(this->Internal->Clips.begin(),
                                 clipsCopy.begin(), clipsCopy.end());
    this->Internal->Cells.insert(this->Internal->Cells.begin(),
                                 clipsCopy.size(), Cell());

    // Move any duplicate clips to the end of the vector, replacing them with
    // their previously loaded (or load-in-progress) twin.
    int numDupes = 0;
    for (size_t i = 0; i < inputSize; ++i)
      {
      for (size_t j = inputSize, end = this->Internal->Clips.size();
           j < end; ++j)
        {
        if (this->Internal->Clips[i].second->GetEventId() ==
            this->Internal->Clips[j].second->GetEventId())
          {
          ++numDupes;
          this->Internal->Clips[i] = this->Internal->Clips[j];
          this->Internal->Cells[i] = this->Internal->Cells[j];
          this->Internal->Cells[j].Renderer = 0;

          if (j + 1 != end)
            {
            // shift the 'hole' from the moved clip to the end of the vector
            ClipVector::iterator begin = this->Internal->Clips.begin();
            std::rotate(begin + j, begin + (j + 1), this->Internal->Clips.end());

            std::vector<Cell>::iterator begin2 = this->Internal->Cells.begin();
            std::rotate(begin2 + j, begin2 + (j + 1), this->Internal->Cells.end());
            }

          // shift input duplicates to the end
          if (i + 1 != inputSize)
            {
            ClipVector::iterator begin = clipsCopy.begin();
            std::rotate(begin + i, begin + (i + 1), clipsCopy.end());
            }

          break;
          }
        }
      }

    // remove duplicate clips, which are now at the end of the vector
    if (numDupes != 0)
      {
      size_t uniques = this->Internal->Clips.size() - numDupes;
      this->Internal->Clips.resize(uniques);

      // if we will be removing cells, make sure to clean up their renderers
      for (size_t i = uniques, end = this->Internal->Cells.size(); i < end; ++i)
        {
        Cell& cell = this->Internal->Cells[i];
        if (cell.Renderer && cell.RendererInitialized)
          {
          this->Widget->GetRenderWindow()->RemoveRenderer(cell.Renderer);
          }
        }

      this->Internal->Cells.resize(uniques);
      clipsCopy.resize(inputSize - numDupes);
      }
    }
  else  // allow duplicates
    {
    this->Internal->Clips.insert(this->Internal->Clips.begin(),
                                 clipsCopy.begin(), clipsCopy.end());
    this->Internal->Cells.insert(this->Internal->Cells.begin(),
                                 inputSize, Cell());
    }

  // drop the 'oldest' clips if there are too many
  if (this->Internal->Clips.size() > MaxClips)
    {
    std::cerr << "Maximum number of clips exceeded - only the first "
              << MaxClips << " will be shown.\n";
    this->Internal->Clips.resize(MaxClips);
    }

  // assign ids to the new clips
  for (size_t i = 0, j = 0; i < inputSize; ++i)
    {
    if (!this->Internal->Cells[i].Used)
      {
      unsigned id = this->CurrentId++;
      this->Internal->Clips[i].first = id;
      this->Internal->Cells[i].Id = id;
      clipsCopy[j++].first = id;
      }
    }

  bool dimensionsChanged = this->UpdateGridDimensions();

  // init new cells or remove excess cells
  size_t numCells = this->GetCellCount();
  size_t numCellsNeeded = this->NumRows * this->NumColumns;
  if (numCellsNeeded != numCells)
    {
    // if we will be removing cells, make sure to clean up their renderers
    for (size_t i = numCellsNeeded; i < numCells; ++i)
      {
      Cell& cell = this->Internal->Cells[i];
      if (cell.RendererInitialized)
        {
        this->Widget->GetRenderWindow()->RemoveRenderer(cell.Renderer);
        }
      }

    this->Internal->Cells.resize(numCellsNeeded);
    for (size_t i = numCells; i < numCellsNeeded; ++i)
      {
      Cell& cell = this->Internal->Cells[i];
      cell.Renderer = vtkSmartPointer<vtkRenderer>::New();
      if (this->HasBeenShown)
        {
        this->InitializeRenderer(cell);
        }
      }
    }

  // update the cells where the input clips will go
  for (size_t i = 0; i < inputSize; ++i)
    {
    // skip duplicates that have already been loaded
    Cell& cell = this->Internal->Cells[i];
    if (cell.Used)
      {
      continue;
      }

    // create renderer
    cell.Used = true;
    cell.Renderer = vtkSmartPointer<vtkRenderer>::New();
    if (this->HasBeenShown)
      {
      this->InitializeRenderer(cell);
      }

    // construct label
    vtkTextActor* TA = vtkTextActor::New();
    TA->GetTextProperty()->SetVerticalJustificationToBottom();
    TA->SetPosition(Padding, Padding);
    cell.Label = TA;
    cell.Renderer->AddViewProp(cell.Label);
    TA->FastDelete();

    // set label text
    this->UpdateCellLabel(cell, this->Internal->Clips[i].second);
    }

  QSize size = this->GetBestSizeAtCurrentScale();
  this->UpdateIdealSize();

  // go ahead and process the clips if the dialog's already visible
  if (this->isVisible())
    {
    // resize the window if the grid size has changed
    if (dimensionsChanged)
      {
      this->resize(this->size().expandedTo(size));
      }

    this->LayoutRenderers();

    if (!clipsCopy.empty())
      {
      this->Internal->ClipBuilder.BuildClips(clipsCopy.begin(), clipsCopy.end());
      }
    }
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::ClipAvailable(vtkImageData* clip, int id)
{
  // find the destination cell
  for (size_t i = 0, end = this->Internal->Cells.size(); i < end; ++i)
    {
    Cell& cell = this->Internal->Cells[i];
    if (cell.Id == id)
      {
      vtkSmartPointer<vtkImageActor> imageActor = vtkSmartPointer<vtkImageActor>::New();
      imageActor->SetInputData(clip);
      clip->Delete();

      vtkRenderer* renderer = cell.Renderer;
      renderer->AddViewProp(imageActor);
      renderer->ResetCamera();
      double* position = renderer->GetActiveCamera()->GetPosition();
      renderer->GetActiveCamera()->SetPosition(position[0], position[1],
                                               clip->GetDimensions()[2] + 2);
      renderer->GetActiveCamera()->SetParallelScale(VideoSize * 0.5);

      cell.Image = imageActor;
      cell.StartTime.start();
      cell.LastUpdate.start();
      this->Widget->GetRenderWindow()->Render();
      return;
      }
    }
  // the target cell has been removed - the clip is DOA
  clip->Delete();
  std::cerr << "Destination cell was removed.\n";
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::LayoutRenderers()
{
  double vidSize = VideoSize * this->VideoScale;

  double w = this->width();
  double h = this->height();

  double xPadding = w - vidSize * this->NumColumns;
  xPadding /= this->NumColumns + 1;

  double yPadding = h - vidSize * this->NumRows;
  yPadding /= this->NumRows + 1;

  int count = 0;
  for (int i = 0; i < this->NumRows; ++i)
    {
    for (int j = 0; j < this->NumColumns; ++j)
      {
      // display first clip in the upper left corner, last in bottom right
      double x0 = xPadding + (vidSize + xPadding) * j;
      double y0 = yPadding + (vidSize + yPadding) * (this->NumRows - 1 - i);
      double x1 = x0 + vidSize;
      double y1 = y0 + vidSize;

      double minViewX = x0 / w;
      double maxViewX = x1 / w;
      double minViewY = y0 / h;
      double maxViewY = y1 / h;

      vtkRenderer* renderer = this->Internal->Cells[count++].Renderer;
      renderer->SetViewport(minViewX, minViewY, maxViewX, maxViewY);
      }
    }
}

//-----------------------------------------------------------------------------
bool vqTrackingClipViewer::UpdateGridDimensions()
{
  int prevNumRows = this->NumRows;
  int prevNumColumns = this->NumColumns;
  int numClips = static_cast<int>(this->GetClipCount());
  this->NumRows = (numClips - 1) / ClipsPerRow + 1;
  this->NumColumns = numClips > ClipsPerRow ? ClipsPerRow : numClips;
  return this->NumRows != prevNumRows || this->NumColumns != prevNumColumns;
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::UpdateIdealSize()
{
  this->IdealSize = this->GetMinWindowSize(1.0, 0);
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::UpdateCellLabel(Cell& cell, vtkVQTrackingClip* clip)
{
  // set label text
  std::ostringstream ostr;
  ostr << clip->GetRank();

  vtkTextProperty* tp = cell.Label->GetTextProperty();

  switch (clip->GetVideoNode()->GetUserScore())
    {
    case vvIqr::PositiveExample:     tp->SetColor(0.3, 1.0, 0.3); break;
    case vvIqr::NegativeExample:     tp->SetColor(1.0, 0.3, 0.3); break;
    case vvIqr::UnclassifiedExample: tp->SetColor(1.0, 1.0, 1.0); break;
    }

  cell.Label->SetInput(ostr.str().c_str());
}

//-----------------------------------------------------------------------------
QSize vqTrackingClipViewer::GetMinWindowSize(double videoScale, int minWidth)
{
  int w = VideoSize * this->NumColumns +
          2 * Padding + Padding * (this->NumColumns - 1);
  int h = VideoSize * this->NumRows +
          2 * Padding + Padding * (this->NumRows - 1);

  return QSize(std::max<double>(w * videoScale, minWidth), h * videoScale);
}

//-----------------------------------------------------------------------------
QSize vqTrackingClipViewer::GetBestSizeAtCurrentScale()
{
  return this->GetMinWindowSize(this->VideoScale);
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::InitializeRenderer(Cell& cell)
{
  vtkCamera* camera = cell.Renderer->GetActiveCamera();
  camera->ParallelProjectionOn();
  camera->SetParallelScale(VideoSize * 0.5);

  // center the video bounds
  double videoCenter = (VideoSize - 1) * 0.5;
  camera->SetPosition(videoCenter, videoCenter, 1.0);
  camera->SetFocalPoint(videoCenter, videoCenter, 0.0);

  cell.Renderer->SetLayer(1);
  this->Widget->GetRenderWindow()->AddRenderer(cell.Renderer);
  cell.RendererInitialized = true;
}

//-----------------------------------------------------------------------------
vtkVQTrackingClip* vqTrackingClipViewer::SelectClipAt(int x, int y,
    bool activate)
{
  vtkRenderer* renderer = this->Widget->GetInteractor()->FindPokedRenderer(x, y);
  if (!renderer)
    {
    return 0;
    }

  // find the cell corresponding to that renderer
  for (size_t i = 0, size = this->Internal->Cells.size(); i < size; ++i)
    {
    Cell& cell = this->Internal->Cells[i];
    if (renderer == cell.Renderer)
      {
      return this->SelectClipInCell(i, activate);
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::RemoveClipAt(int x, int y)
{
  if (this->GetClipCount() < 2)
    {
    return;
    }

  vtkRenderer* renderer = this->Widget->GetInteractor()->FindPokedRenderer(x, y);
  if (!renderer)
    {
    return;
    }

  // find the cell corresponding to that renderer
  for (size_t i = 0, size = this->Internal->Cells.size(); i < size; ++i)
    {
    Cell& cell = this->Internal->Cells[i];
    if (renderer == cell.Renderer)
      {
      this->RemoveClipInCell(i);
      return;
      }
    }
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::UpdateClips(vqCore* core)
{
  for (size_t i = 0; i < this->Internal->Clips.size();)
    {
    vtkVgVideoNode* node =
      core->getResultNode(this->Internal->Clips[i].second->GetEventId());

    // remove clip if the result is no longer available
    if (!node)
      {
      this->RemoveClipInCell(i);
      continue;
      }

    // node for this id has probably changed
    this->Internal->Clips[i].second->SetVideoNode(node);

    // update the cell label if the rank of this result has changed
    int rank = node->GetRank();
    if (rank != this->Internal->Clips[i].second->GetRank())
      {
      this->Internal->Clips[i].second->SetRank(rank);

      this->UpdateCellLabel(this->Internal->Cells[i],
                            this->Internal->Clips[i].second);
      }

    ++i;
    }
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::RefreshClips()
{
  for (size_t i = 0; i < this->Internal->Clips.size(); ++i)
    {
    this->UpdateCellLabel(this->Internal->Cells[i],
                          this->Internal->Clips[i].second);
    }
}

//-----------------------------------------------------------------------------
vtkVQTrackingClip* vqTrackingClipViewer::SelectClipInCell(size_t cellIndex,
    bool activate)
{
  Cell& cell = this->Internal->Cells[cellIndex];

  // find and select the corresponding clip
  for (ClipVector::iterator itr = this->Internal->Clips.begin();
       itr != this->Internal->Clips.end();
       ++itr)
    {
    if (itr->first == cell.Id)
      {
      emit ClipSelected(itr->second->GetEventId());
      if (activate)
        {
        emit ClipActivated(itr->second->GetEventId());
        }
      return itr->second;
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::RemoveClipInCell(size_t cellIndex)
{
  Cell& cell = this->Internal->Cells[cellIndex];

  // find and delete the corresponding clip
  for (ClipVector::iterator itr = this->Internal->Clips.begin();
       itr != this->Internal->Clips.end();
       ++itr)
    {
    if (itr->first == cell.Id)
      {
      // tell the clip builder not to process this clip if it hasn't already
      this->Internal->ClipBuilder.SkipClip(itr->first);
      this->Internal->Clips.erase(itr);
      break;
      }
    }

  // 'clear' the cell
  if (cell.Image)
    {
    cell.Renderer->RemoveViewProp(cell.Image);
    cell.Image = 0;
    }
  if (cell.Label)
    {
    cell.Renderer->RemoveViewProp(cell.Label);
    cell.Label = 0;
    }
  cell.Id = -1;
  cell.StartTime = QTime();
  cell.LastUpdate = QTime();

  // move the erased cell to the last position
  if (cellIndex + 1 != this->Internal->Cells.size())
    {
    std::rotate(this->Internal->Cells.begin() + cellIndex,
                this->Internal->Cells.begin() + cellIndex + 1,
                this->Internal->Cells.end());
    }

  if (this->UpdateGridDimensions())
    {
    // remove dead renderers
    size_t gridSize = this->NumRows * this->NumColumns;
    for (size_t i = gridSize, end = this->Internal->Cells.size(); i < end; ++i)
      {
      this->Widget->GetRenderWindow()
      ->RemoveRenderer(this->Internal->Cells[i].Renderer);
      }
    this->Internal->Cells.resize(gridSize);

    // shrink the window
    QSize size = this->GetBestSizeAtCurrentScale();
    this->UpdateIdealSize();
    this->resize(size);
    }

  this->LayoutRenderers();
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::RequestNextClips()
{
  emit this->RequestNextClips(
    this->Internal->Clips.back().second->GetVideoNode()->GetInstanceId(),
    static_cast<int>(this->GetClipCount()));
}

//-----------------------------------------------------------------------------
void vqTrackingClipViewer::RemoveAllClips()
{
  size_t numClips = this->GetClipCount();
  for (size_t i = 0; i < numClips; ++i)
    {
    this->RemoveClipInCell(0);
    }
}
