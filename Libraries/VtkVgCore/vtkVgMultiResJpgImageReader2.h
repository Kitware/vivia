/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// .NAME vtkVgMultiResJpgImageReader2 - Implements an api for reading image LODs
// .SECTION Description
// .SECTION see also

#ifndef __vtkVgMultiResJpgImageReader2_h
#define __vtkVgMultiResJpgImageReader2_h

#include "vtkVgBaseImageSource.h"

#include <vgExport.h>

class vtkImageData;

class VTKVG_CORE_EXPORT vtkVgMultiResJpgImageReader2
  : public vtkVgBaseImageSource
{
public:
  // Description:
  // Create new instance.
  static vtkVgMultiResJpgImageReader2* New();

  // Description:
  // Check type.
  vtkTypeMacro(vtkVgMultiResJpgImageReader2, vtkVgBaseImageSource);

  // Description:
  // Print values of the member variables.
  void PrintSelf(ostream& os, vtkIndent indent);

  // @NOTE: Commented for now.
  // Description:
  // Pattern used in sprintf to create file name.
  // Example: "C:/Image_%05d.mrj"
  // First parameter is the frame.
  // vtkSetStringMacro(FilePattern);
  //  vtkGetStringMacro(FilePattern);

  // Description:

  // Description:
  // ScreenPixelSize/ImagePixelSize.
  vtkSetMacro(Scale, double);
  vtkGetMacro(Scale, double);

  // @NOTE: Commented for now.
  // Description:
  // Which video frame to load.
  //  vtkSetMacro(Frame,int);
  //  vtkGetMacro(Frame,int);

  // Description:
  // This flag is on by default.
  // When it is off, the image returned may be larger than
  // the requested crop extent.
  vtkSetMacro(ExactExtent, int);
  vtkGetMacro(ExactExtent, int);
  vtkBooleanMacro(ExactExtent, int);

  // Description:
  // Set the level number for the level of detail.
  vtkSetMacro(Level, int);
  vtkGetMacro(Level, int);

  // Description:
  // MetaData filled in after UpdateInformation is called.
  vtkGetVector2Macro(Dimensions, vtkTypeUInt32);
  vtkGetVector2Macro(TileDimensions, vtkTypeUInt32);
  virtual void GetDimensions(int dims[2]);
  void GetTileDimensions(int dims[2]);

  // Description:
  // Return the number of level of details.
  virtual int GetNumberOfLevels() const;

  virtual bool CanRead(const std::string& source) const;

  virtual std::string GetShortDescription() const;

  virtual std::string GetLongDescription() const;

  static vtkVgBaseImageSource* Create();


protected:
  vtkVgMultiResJpgImageReader2();
  ~vtkVgMultiResJpgImageReader2();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual void ExecuteDataWithInformation(vtkDataObject* data, vtkInformation* outInfo);

  void ComputeLevel();

private:

  int ExactExtent;

  // @NOTE: Commented for now.
  //  char* FilePattern;

  int Frame;


  unsigned char NumberOfLevels;
  vtkTypeUInt32 Dimensions[2];

//  char* FileName;
  vtkTypeUInt32 TileDimensions[2];
  vtkTypeUInt32 LevelTableLocation;
};

#endif
