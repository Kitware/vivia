/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgJPEGMemoryReader - read JPEG files
// .SECTION Description
// vtkVgJPEGMemoryReader is a source object that reads JPEG files.
// It should be able to read most any JPEG file
//
// .SECTION See Also
// vtkJPEGWriter

#ifndef __vtkVgJPEGMemoryReader_h
#define __vtkVgJPEGMemoryReader_h

#include "vtkImageReader2.h"

#include <vgExport.h>

#include <cassert>

class VTKVG_CORE_EXPORT vtkVgJPEGMemoryReader : public vtkImageReader2
{
public:
  static vtkVgJPEGMemoryReader* New();
  vtkTypeMacro(vtkVgJPEGMemoryReader, vtkImageReader2);
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  virtual void SetMemoryBuffer(void*) {assert(false);}
  void SetMemoryBuffer(unsigned char* buff, int size)
    {
    this->Modified();
    this->Buffer = buff;
    this->BufferSize = size;
    }
protected:
  vtkVgJPEGMemoryReader() {this->Buffer = 0; this->BufferSize = 0;};
  ~vtkVgJPEGMemoryReader() {};

  virtual void ExecuteInformation();
  virtual void ExecuteDataWithInformation(vtkDataObject* out, vtkInformation* outInfo);
public:
  unsigned char* Buffer;
  int BufferSize;
private:
  vtkVgJPEGMemoryReader(const vtkVgJPEGMemoryReader&);  // Not implemented.
  void operator=(const vtkVgJPEGMemoryReader&);  // Not implemented.
};
#endif
