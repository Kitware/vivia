/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgImageSource_h
#define __vtkVgImageSource_h

#include "vtkVgBaseImageSource.h"

#include <vgExport.h>

class vtkVgImageSourceInternal;

class VTKVG_CORE_EXPORT vtkVgImageSource : public vtkVgBaseImageSource
{
public:

  static vtkVgImageSource* New();

  vtkTypeMacro(vtkVgImageSource, vtkVgBaseImageSource);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void GetDimensions(int dim[2]);

  const int* GetFullImageExtents();

  virtual bool CanRead(const std::string& source);

  virtual std::string GetShortDescription();

  virtual std::string GetLongDescription();

  static vtkVgBaseImageSource* Create();


protected:

  vtkVgImageSource();
  virtual ~vtkVgImageSource();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:

  vtkVgImageSource(const vtkVgImageSource&);  // Not implemented.
  void operator=(const vtkVgImageSource&);    // Not implemented.


// Data members.
private:

  int   FullImageExtents[4];

  vtkVgImageSourceInternal* Implementation;
};

#endif // __vtkVgImageSource_h
