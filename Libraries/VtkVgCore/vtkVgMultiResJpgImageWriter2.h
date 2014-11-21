/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgMultiResJpgImageWriter2 - Writes images to files.
// .SECTION Description

#ifndef __vtkVgMultiResJpgImageWriter2_h
#define __vtkVgMultiResJpgImageWriter2_h

#include "vtkImageAlgorithm.h"

#include <vgExport.h>

class VTKVG_CORE_EXPORT vtkVgMultiResJpgImageWriter2 : public vtkImageAlgorithm
{
public:
  static vtkVgMultiResJpgImageWriter2* New();
  vtkTypeMacro(vtkVgMultiResJpgImageWriter2, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name for the image file. You should specify either
  // a FileName or a FilePrefix. Use FilePrefix if the data is stored
  // in multiple files.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

//BTX
  // Description:
  // Set/Get the input object from the image pipeline.
  vtkImageData* GetInput();
//ETX

  // Description:
  // Defaults to 5 levels.
  vtkSetMacro(NumberOfLevels, int);
  vtkGetMacro(NumberOfLevels, int);

  // Description:
  // Set/Get compression quality for the jpeg.
  vtkSetMacro(CompressionQuality, int);
  vtkGetMacro(CompressionQuality, int);

  // Description:
  // All tiles will have the same dimension.
  // Somes tiles at the edge may have to be padded with 0's.
  vtkSetVector2Macro(TileDimensions, int);
  vtkGetVector2Macro(TileDimensions, int);

  //void DeleteFiles();

  void Write();

protected:
  vtkVgMultiResJpgImageWriter2();
  ~vtkVgMultiResJpgImageWriter2();

  char* FileName;

  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  int WriteImage(vtkImageData* data);
  int WriteLevel(FILE* fp, vtkImageData* input);


private:
  vtkVgMultiResJpgImageWriter2(const vtkVgMultiResJpgImageWriter2&);  // Not implemented.
  void operator=(const vtkVgMultiResJpgImageWriter2&);  // Not implemented.

  int NumberOfLevels;
  int CompressionQuality;
  int TileDimensions[2];
};

#endif
