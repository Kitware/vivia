// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvClipVideoRepresentation_h
#define __vvClipVideoRepresentation_h

// Qt includes.
#include <QObject>

#include <vgExport.h>

// VG includes.
#include "vtkVgVideoRepresentation0.h"

// Forward declarations.
class vtkVgEvent;
struct vtkVgVideoFrameData;

class VV_VTKWIDGETS_EXPORT vvClipVideoRepresentation
  : public QObject, public vtkVgVideoRepresentation0
{
  Q_OBJECT

public:
  // Description:
  // Define easy to use types.
  vtkVgClassMacro(vvClipVideoRepresentation);

  vtkTypeMacro(vvClipVideoRepresentation, vtkVgVideoRepresentation0);

  static vvClipVideoRepresentation* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(AutoCenter, int);
  vtkGetMacro(AutoCenter, int);

  virtual void Update();

  virtual void AutoCenterUpdate(const vtkVgVideoFrameData* frameData);

signals:
  void areaOfInterest(double*, double, double);

protected:
  int AutoCenter;

  double RegionsMaxWidth;
  double RegionsMaxHeight;

  vtkVgEvent* VGEventCached;

protected:
  vvClipVideoRepresentation();
  virtual ~vvClipVideoRepresentation();

  virtual void HandleModelError() {}
  virtual void HandleAnimationCueTickEvent();
private:
  // Not implemented.
  vvClipVideoRepresentation(const vvClipVideoRepresentation&);
  void operator= (const vvClipVideoRepresentation&);
};

#endif // __vvClipVideoRepresentation_h
