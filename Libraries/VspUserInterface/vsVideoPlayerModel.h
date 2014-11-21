/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
