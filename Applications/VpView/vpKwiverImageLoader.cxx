/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpKwiverImageLoader.h"

#include "vpImageSourceFactory.h"

#include <vtkImageData.h>

#include <vital/types/image_container.h>

#include <qtIndexRange.h>

namespace kv = kwiver::vital;

namespace // anonymous
{

//-----------------------------------------------------------------------------
class ImageConverterBase
{
public:
  virtual ~ImageConverterBase() = default;

  virtual kv::image_pixel_traits pixelTraits() const = 0;
  virtual void copyImageData(vtkImageData* in, kv::image& out) const = 0;
};

//-----------------------------------------------------------------------------
template <typename T>
class ImageConverter : public ImageConverterBase
{
public:
  virtual kv::image_pixel_traits pixelTraits() const override;
  virtual void copyImageData(vtkImageData* in, kv::image& out) const override;
};

//-----------------------------------------------------------------------------
template <typename T>
kv::image_pixel_traits ImageConverter<T>::pixelTraits() const
{
  return kv::image_pixel_traits_of<T>{};
}

//-----------------------------------------------------------------------------
template <typename T>
void ImageConverter<T>::copyImageData(vtkImageData* in, kv::image& out) const
{
  const auto w = out.width();
  const auto h = out.height();
  const auto d = out.depth();

  const auto lp = w * d;
  const auto lb = lp * sizeof(T);

  Q_ASSERT(out.h_step == lb);
  Q_ASSERT(out.d_step == sizeof(T));
  Q_ASSERT(out.w_step == sizeof(T) * d);

  // Copy image data by scanlines, working from bottom up to account for VTK's
  // Y-inverted coordinate system
  auto* inp = static_cast<T*>(in->GetScalarPointer()) + (lp * (h - 1));
  auto* outp = static_cast<T*>(out.first_pixel());

  for (const auto i : qtIndexRange(h))
  {
    Q_UNUSED(i);

    memcpy(outp, inp, lb);
    inp -= lp;
    outp += lp;
  }
}

//-----------------------------------------------------------------------------
std::unique_ptr<ImageConverterBase> traitsFromVtkType(int vtkType)
{
  using p = std::unique_ptr<ImageConverterBase>;

  switch (vtkType)
  {
    case VTK_CHAR:
      return p{new ImageConverter<char>};
    case VTK_SIGNED_CHAR:
      return p{new ImageConverter<signed char>};
    case VTK_UNSIGNED_CHAR:
      return p{new ImageConverter<unsigned char>};
    case VTK_SHORT:
      return p{new ImageConverter<signed short>};
    case VTK_UNSIGNED_SHORT:
      return p{new ImageConverter<unsigned short>};
    case VTK_INT:
      return p{new ImageConverter<signed int>};
    case VTK_UNSIGNED_INT:
      return p{new ImageConverter<unsigned int>};
    case VTK_LONG:
      return p{new ImageConverter<signed long>};
    case VTK_UNSIGNED_LONG:
      return p{new ImageConverter<unsigned long>};
    case VTK_LONG_LONG:
      return p{new ImageConverter<signed long long>};
    case VTK_UNSIGNED_LONG_LONG:
      return p{new ImageConverter<unsigned long long>};
    case VTK_FLOAT:
      return p{new ImageConverter<float>};
    case VTK_DOUBLE:
      return p{new ImageConverter<double>};
    default:
      return nullptr;
  }
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vpKwiverImageLoaderPrivate
{
public:
  vtkSmartPointer<vtkVgBaseImageSource> imageSource;
};

QTE_IMPLEMENT_D_FUNC(vpKwiverImageLoader)

//-----------------------------------------------------------------------------
vpKwiverImageLoader::vpKwiverImageLoader() :
  d_ptr{new vpKwiverImageLoaderPrivate}
{
}

//-----------------------------------------------------------------------------
vpKwiverImageLoader::~vpKwiverImageLoader()
{
}

//-----------------------------------------------------------------------------
kv::image_container_sptr vpKwiverImageLoader::load(const std::string& filename)
{
  QTE_D();

  // Create reader, if not already created
  if (!d->imageSource)
  {
    auto* const factory = vpImageSourceFactory::GetInstance();
    d->imageSource.TakeReference(factory->Create(filename));
    if (!d->imageSource)
    {
      return nullptr;
    }
  }

  // Read image data from file
  d->imageSource->SetFileName(filename.c_str());
  d->imageSource->Update();
  auto* const data = d->imageSource->GetOutput();

  // Get input image data dimensions and number of components
  int dim[3];
  data->GetDimensions(dim);
  const auto nc = data->GetNumberOfScalarComponents();

  // Get converter for scalar type
  auto converter = traitsFromVtkType(data->GetScalarType());
  if (!converter || dim[0] <= 0 || dim[1] <= 0 || dim[2] != 1 || nc <= 0)
  {
    return nullptr;
  }

  // Create output image
  kv::image image{static_cast<size_t>(dim[0]), static_cast<size_t>(dim[1]),
                  static_cast<size_t>(nc), true, converter->pixelTraits()};

  // Copy image data and return output image
  converter->copyImageData(data, image);
  return std::make_shared<kv::simple_image_container>(image);
}
