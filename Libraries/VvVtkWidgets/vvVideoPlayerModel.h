// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvVideoPlayerModel_h
#define __vvVideoPlayerModel_h

// QT includes.
#include <QObject>

// VG includes.
#include <vtkVgVideoModel0.h>

// VTK includes.
#include <vtkSmartPointer.h>

#include <vgExport.h>

class VV_VTKWIDGETS_EXPORT vvVideoPlayerModel
  : public QObject, public vtkVgVideoModel0
{
  Q_OBJECT

public:
  vtkVgClassMacro(vvVideoPlayerModel);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vvVideoPlayerModel, vtkVgVideoModel0);

  static vvVideoPlayerModel* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get if this model is sharing a video source with other
  // model. If yes how the data get fed to the model is different
  // than when its not sharing.
  vtkSetMacro(SharingSource, int);
  vtkGetMacro(SharingSource, int);

  // Description:
  // Overridden functions.
  virtual int Update(const vtkVgTimeStamp& timeStamp,
                     const vtkVgTimeStamp* referenceFrameTimeStamp);
  using Superclass::Update;

signals:

  void FrameAvailable();

public slots:
  void OnFrameAvailable();

protected:
  vvVideoPlayerModel();
  virtual  ~vvVideoPlayerModel();

  int       SharingSource;

private:

  vvVideoPlayerModel(const vvVideoPlayerModel&);
  void operator=(const vvVideoPlayerModel&);
};

#endif // __vvVideoPlayerModel_h
