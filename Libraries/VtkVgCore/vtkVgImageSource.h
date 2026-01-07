// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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

  virtual bool CanRead(const std::string& source) const;

  virtual std::string GetShortDescription() const;

  virtual std::string GetLongDescription() const;

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
