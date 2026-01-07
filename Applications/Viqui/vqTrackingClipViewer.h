// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqTrackingClipViewer_h
#define __vqTrackingClipViewer_h

#include <vtkSmartPointer.h>

#include <QDialog>

class QVTKWidget;
class vqCore;
class vtkImageActor;
class vtkImageData;
class vtkRenderer;
class vtkInteractorCallback;
class vtkVQTrackingClip;

class vqTrackingClipViewer : public QDialog
{
  Q_OBJECT

public:
  // id, clip pair
  typedef std::pair< int, vtkSmartPointer<vtkVQTrackingClip> > ClipElement;
  typedef std::vector<ClipElement> ClipVector;

public:
  vqTrackingClipViewer(QWidget* parent = 0);
  virtual ~vqTrackingClipViewer();

  void InsertClips(const ClipVector& clips, bool allowDuplicates = false);

  vtkVQTrackingClip* SelectClipAt(int x, int y, bool activate = false);
  void RemoveClipAt(int x, int y);

  void RemoveAllClips();
  void UpdateClips(vqCore* core);
  void RefreshClips();
  void RequestNextClips();

  virtual void showEvent(QShowEvent* event);
  virtual void hideEvent(QHideEvent* event);
  virtual void closeEvent(QCloseEvent* event);
  virtual void resizeEvent(QResizeEvent* event);

  QVTKWidget* GetWidget()  { return this->Widget; }

  void SetAllowFrameSkip(bool enable)
    { this->AllowFrameSkip = enable; }

  bool GetAllowFrameSkip()
    { return this->AllowFrameSkip; }

signals:
  void ClipSelected(int id);
  void ClipActivated(int id);
  void ClipRatingChanged(int id, int rating);
  void RequestNextClips(vtkIdType previousId, int count);

public slots:
  void ClipAvailable(vtkImageData* clip, int id);

protected:
  struct Cell;

  size_t GetClipCount();
  size_t GetCellCount();
  Cell&  GetCell(size_t index);

  void LayoutRenderers();
  void InitializeRenderer(Cell& cell);

  bool UpdateGridDimensions();
  void UpdateIdealSize();
  void UpdateCellLabel(Cell& cell, vtkVQTrackingClip* clip);

  QSize GetMinWindowSize(double videoScale = 1.0, int minWidth = 150);
  QSize GetBestSizeAtCurrentScale();

  vtkVQTrackingClip* SelectClipInCell(size_t cellIndex, bool activate);
  void RemoveClipInCell(size_t cellIndex);

private:
  vqTrackingClipViewer(const vqTrackingClipViewer&); // Not implemented.
  void operator=(const vqTrackingClipViewer&);       // Not implemented.

  vtkSmartPointer<vtkInteractorCallback> InteractorCallback;

  QVTKWidget* Widget;

  bool HasBeenShown;
  bool AllowFrameSkip;

  int NumRows, NumColumns;

  QSize IdealSize;

  double VideoScale;

  class vqInternal;
  vqInternal* Internal;

  unsigned CurrentId;

  friend class vtkInteractorCallback;
};

#endif // __vqTrackingClipViewer_h
