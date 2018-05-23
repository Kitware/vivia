/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVgImageBuffer_h
#define __vtkVgImageBuffer_h

#include <vgExport.h>

#include <array>
#include <functional>
#include <memory>

#include <cstdint>

template <typename T> class vtkSmartPointer;

class vtkImageData;

class VTKVG_CORE_EXPORT vtkVgImageBuffer
{
public:
  vtkVgImageBuffer(int vtkPixelType) : PixelType{vtkPixelType} {}
  ~vtkVgImageBuffer() = default;

  int GetPixelType() const { return this->PixelType; }
  size_t GetScalarSize() const;
  size_t GetSize() const;
  const char* GetData() const { return this->Data; }
  char* GetMutableData() const { return this->MutableData; }

  std::array<size_t, 3> GetDimensions() const { return this->Dimensions; }
  std::array<ptrdiff_t, 3> GetStrides() const { return this->Strides; }

  vtkSmartPointer<vtkImageData> Convert() const;
  vtkSmartPointer<vtkImageData> Convert(
    std::function<uint8_t(uint8_t)> convertPixel) const;
  vtkSmartPointer<vtkImageData> Convert(
    std::function<uint16_t(uint16_t)> convertPixel) const;

  void SetLayout(size_t iSize, size_t jSize, size_t kSize,
                 ptrdiff_t iStride, ptrdiff_t jStride, ptrdiff_t kStride);

  void Allocate();
  void SetData(const char*);

protected:
  const int PixelType;

  std::array<size_t, 3> Dimensions;
  std::array<ptrdiff_t, 3> Strides;

  std::unique_ptr<char[]> AllocatedData;
  const char* Data = nullptr;
  char* MutableData = nullptr;

private:
  vtkVgImageBuffer(const vtkVgImageBuffer&) = delete;
  vtkVgImageBuffer& operator=(const vtkVgImageBuffer&) = delete;
};

#endif
