// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsVideoPlayerModel_h
#define __vsVideoPlayerModel_h

// VG includes.
#include <vvVideoPlayerModel.h>

class vsVideoPlayerModel : public vvVideoPlayerModel
{
  vtkVgClassMacro(vsVideoPlayerModel);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vsVideoPlayerModel, vvVideoPlayerModel);

  static vsVideoPlayerModel* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Overridden functions.
  virtual int Play();

  virtual int Pause();

  virtual int Stop();

protected:
  vsVideoPlayerModel();
  virtual ~vsVideoPlayerModel();

private:
  vsVideoPlayerModel(const vsVideoPlayerModel&);
  void operator=(const vsVideoPlayerModel&);
};

#endif
