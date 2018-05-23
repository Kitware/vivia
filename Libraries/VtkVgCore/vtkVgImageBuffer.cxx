/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgImageBuffer.h"

#include <vtkImageData.h>
#include <vtkSmartPointer.h>

#include <qtIndexRange.h>

namespace
{

//-----------------------------------------------------------------------------
template <typename T>
vtkSmartPointer<vtkImageData> convert(
  const vtkVgImageBuffer& buffer, std::function<T(T)> convertPixel = nullptr)
{
  // Get layout
  const auto& dim = buffer.GetDimensions();
  const auto& stride = buffer.GetStrides();
  const auto ss = buffer.GetScalarSize();

  // Compute address of first pixel
  const char* src = buffer.GetData();
  src += (stride[0] > 0 ? 0 : -stride[0] * static_cast<ptrdiff_t>(dim[0] - 1));
  src += (stride[1] > 0 ? 0 : -stride[1] * static_cast<ptrdiff_t>(dim[1] - 1));
  src += (stride[2] > 0 ? 0 : -stride[2] * static_cast<ptrdiff_t>(dim[2] - 1));

  // Allocate destination image data
  auto out = vtkSmartPointer<vtkImageData>::New();
  out->SetExtent(0, static_cast<int>(dim[0] - 1),
                 0, static_cast<int>(dim[1] - 1), 0, 0);
  out->SetSpacing(1.0, 1.0, 1.0);
  out->AllocateScalars(buffer.GetPixelType(), static_cast<int>(dim[2]));
  auto* const dst = reinterpret_cast<T*>(out->GetScalarPointer());

  // Copy data
  const auto scanlineSize = dim[0] * dim[2];
  if (convertPixel)
    {
    // A conversion function is specified; we must iterate over each scalar
    for (auto j : qtIndexRange(dim[1]))
      {
      const auto jos = static_cast<ptrdiff_t>(j) * stride[1];
      const auto jod = static_cast<ptrdiff_t>(j * scanlineSize);
      for (auto i : qtIndexRange(dim[0]))
        {
        const auto ios = static_cast<ptrdiff_t>(i) * stride[0] + jos;
        const auto iod = static_cast<ptrdiff_t>(i * dim[2]) + jod;
        for (auto k : qtIndexRange(dim[2]))
          {
          const auto kos = static_cast<ptrdiff_t>(k) * stride[2];
          dst[iod + static_cast<ptrdiff_t>(k)] =
            convertPixel(*reinterpret_cast<const T*>(src + ios + kos));
          }
        }
      }
    }
  else if (stride[2] == static_cast<ptrdiff_t>(ss) &&
           stride[0] == static_cast<ptrdiff_t>(ss * dim[2]))
    {
    // Image data is packed by all scalars of a pixel, then by rows, which is
    // how vtkImageData wants to be packed; we can import data by scanlines
    for (auto j : qtIndexRange(dim[1]))
      {
      const auto jos = static_cast<ptrdiff_t>(j) * stride[1];
      const auto jod = static_cast<ptrdiff_t>(j * scanlineSize);
      memcpy(dst + jod, src + jos, scanlineSize);
      }
    }
  else
    {
    // Image data is packed in some other manner; we must iterate over each
    // scalar (further optimization for packing by pixel then column would be
    // possible, but this ordering is virtually unheard-of; more likely we
    // are packed by row, then column, with all scalars for a single channel
    // packed in a block, which is the worst case)
    for (auto j : qtIndexRange(dim[1]))
      {
      const auto jos = static_cast<ptrdiff_t>(j) * stride[1];
      const auto jod = static_cast<ptrdiff_t>(j * scanlineSize);
      for (auto i : qtIndexRange(dim[0]))
        {
        const auto ios = static_cast<ptrdiff_t>(i) * stride[0] + jos;
        const auto iod = static_cast<ptrdiff_t>(i * dim[2]) + jod;
        for (auto k : qtIndexRange(dim[2]))
          {
          const auto kos = static_cast<ptrdiff_t>(k) * stride[2];
          dst[iod + static_cast<ptrdiff_t>(k)] =
            *reinterpret_cast<const T*>(src + ios + kos);
          }
        }
      }
    }

  return out;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
size_t vtkVgImageBuffer::GetScalarSize() const
{
  switch (this->PixelType)
    {
    case VTK_CHAR:                return sizeof(char);
    case VTK_SIGNED_CHAR:         return sizeof(signed char);
    case VTK_UNSIGNED_CHAR:       return sizeof(unsigned char);
    case VTK_SHORT:               return sizeof(signed short);
    case VTK_UNSIGNED_SHORT:      return sizeof(unsigned short);
    case VTK_INT:                 return sizeof(signed int);
    case VTK_UNSIGNED_INT:        return sizeof(unsigned int);
    case VTK_LONG:                return sizeof(signed long);
    case VTK_UNSIGNED_LONG:       return sizeof(unsigned long);
    case VTK_LONG_LONG:           return sizeof(signed long long);
    case VTK_UNSIGNED_LONG_LONG:  return sizeof(unsigned long long);
    case VTK_FLOAT:               return sizeof(float);
    case VTK_DOUBLE:              return sizeof(double);
    default:                      return 0;
    }
}

//-----------------------------------------------------------------------------
size_t vtkVgImageBuffer::GetSize() const
{
  if (this->Strides[0] > this->Strides[1])
    {
    if (this->Strides[0] > this->Strides[2])
      {
      // 0 is largest stride
      const auto is = static_cast<size_t>(abs(this->Strides[0]));
      return is * this->Dimensions[0];
      }
    }
  else if (this->Strides[1] > this->Strides[2])
    {
    // 1 is largest stride
    const auto js = static_cast<size_t>(abs(this->Strides[1]));
    return js * this->Dimensions[1];
    }

  // 2 is largest stride
  const auto ks = static_cast<size_t>(abs(this->Strides[2]));
  return ks * this->Dimensions[2];
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vtkVgImageBuffer::Convert() const
{
  switch (this->PixelType)
    {
    case VTK_CHAR:                return convert<char>(*this);
    case VTK_SIGNED_CHAR:         return convert<signed char>(*this);
    case VTK_UNSIGNED_CHAR:       return convert<unsigned char>(*this);
    case VTK_SHORT:               return convert<signed short>(*this);
    case VTK_UNSIGNED_SHORT:      return convert<unsigned short>(*this);
    case VTK_INT:                 return convert<signed int>(*this);
    case VTK_UNSIGNED_INT:        return convert<unsigned int>(*this);
    case VTK_LONG:                return convert<signed long>(*this);
    case VTK_UNSIGNED_LONG:       return convert<unsigned long>(*this);
    case VTK_LONG_LONG:           return convert<signed long long>(*this);
    case VTK_UNSIGNED_LONG_LONG:  return convert<unsigned long long>(*this);
    case VTK_FLOAT:               return convert<float>(*this);
    case VTK_DOUBLE:              return convert<double>(*this);
    default:                      break;
    }
  std::cerr << "Cannot convert image buffer: pixel type mismatch\n";
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vtkVgImageBuffer::Convert(
  std::function<uint8_t(uint8_t)> convertPixel) const
{
  if (this->PixelType != VTK_TYPE_UINT8)
    {
    std::cerr << "Cannot convert image buffer: pixel type mismatch\n";
    return nullptr;
    }
  return convert(*this, convertPixel);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> vtkVgImageBuffer::Convert(
  std::function<uint16_t(uint16_t)> convertPixel) const
{
  if (this->PixelType != VTK_TYPE_UINT16)
    {
    std::cerr << "Cannot convert image buffer: pixel type mismatch\n";
    return nullptr;
    }
  return convert(*this, convertPixel);
}

//-----------------------------------------------------------------------------
void vtkVgImageBuffer::SetLayout(
  size_t iSize, size_t jSize, size_t kSize,
  ptrdiff_t iStride, ptrdiff_t jStride, ptrdiff_t kStride)
{
  this->Dimensions[0] = iSize;
  this->Dimensions[1] = jSize;
  this->Dimensions[2] = kSize;
  this->Strides[0] = iStride;
  this->Strides[1] = jStride;
  this->Strides[2] = kStride;
}

//-----------------------------------------------------------------------------
void vtkVgImageBuffer::Allocate()
{
  this->AllocatedData.reset(new char[this->GetSize()]);
  this->Data = this->MutableData = this->AllocatedData.get();
}

//-----------------------------------------------------------------------------
void vtkVgImageBuffer::SetData(const char* data)
{
  this->AllocatedData.reset();
  this->MutableData = nullptr;
  this->Data = data;
}
