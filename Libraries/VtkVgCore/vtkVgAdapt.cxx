// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgAdapt.h"

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>

//-----------------------------------------------------------------------------
void vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>& in, vtkMatrix4x4* out)
{
  double k = (in[2][2] < 0.0 ? -1.0 : 1.0);

  out->Identity();

  out->SetElement(0, 0, in[0][0] * k);
  out->SetElement(0, 1, in[0][1] * k);
  out->SetElement(0, 3, in[0][2] * k);

  out->SetElement(1, 0, in[1][0] * k);
  out->SetElement(1, 1, in[1][1] * k);
  out->SetElement(1, 3, in[1][2] * k);

  out->SetElement(3, 0, in[2][0] * k);
  out->SetElement(3, 1, in[2][1] * k);
  out->SetElement(3, 3, in[2][2] * k);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4>
vtkVgAdapt(const vnl_matrix_fixed<double, 3, 3>& vnlMatrix)
{
  vtkSmartPointer<vtkMatrix4x4> vtkMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();

  vtkVgAdapt(vnlMatrix, vtkMatrix);

  return vtkMatrix;
}

//-----------------------------------------------------------------------------
void vtkVgAdapt(const vgMatrix4d& in, vtkMatrix4x4* out)
{
  std::memcpy(out->GetData(), in.data(), 16 * sizeof(double));
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> vtkVgAdapt(const vgMatrix4d& eigenMatrix)
{
  vtkSmartPointer<vtkMatrix4x4> vtkMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();

  vtkVgAdapt(eigenMatrix, vtkMatrix);

  return vtkMatrix;
}

//-----------------------------------------------------------------------------
vgMatrix4d vtkVgAdapt(const vtkMatrix4x4* in)
{
  // FIXME(VTK9)
  return vgMatrix4d{const_cast<vtkMatrix4x4*>(in)->GetData()};
}

//-----------------------------------------------------------------------------
void vtkVgAdapt(const vil_image_view<double>& img, vtkImageData* data)
{
  unsigned int nc = img.nplanes();

  int* dim = data->GetDimensions();
  if (dim[0] != img.ni() || dim[1] != img.nj() ||
      data->GetNumberOfScalarComponents() != nc ||
      data->GetScalarType() != VTK_DOUBLE)
    {
    data->SetDimensions(img.ni(), img.nj(), 1);
    data->AllocateScalars(VTK_DOUBLE, nc);
    }

  // vtk images have their origin at the bottom left, so we flip the y axis
  double* vtkptr = static_cast<double*>(data->GetScalarPointer());
  vtkptr += img.ni() * (img.nj() - 1) * nc;

  const double* vxlptr = img.top_left_ptr();
  ptrdiff_t istp = img.istep(), jstp = img.jstep();
  ptrdiff_t pstep = img.planestep();
  ptrdiff_t vtkrstep = nc * img.ni();
  for (unsigned int p = 0; p < nc; p++, vxlptr += pstep, ++vtkptr)
    {
    const double* vxlpl = vxlptr;
    double* vtkch = vtkptr;
    for (unsigned int j = 0; j < img.nj(); j++, vxlpl += jstp, vtkch -= vtkrstep)
      {
      const double* vxlrow = vxlpl;
      double* vtkrow = vtkch;
      for (unsigned int i = 0; i < img.ni(); i++, vxlrow += istp, vtkrow += nc)
        {
        *vtkrow = *vxlrow;
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkVgAdapt(vtkImageData* data, vil_image_view<vxl_byte>& img)
{
  if (data->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
    std::cerr << "Input vtkImageData must be of type unsigned char\n";
    }

  unsigned int numberOfComponents =
    static_cast<unsigned int>(data->GetNumberOfScalarComponents());
  if (numberOfComponents != 1)
    {
    std::cerr << "Currently only support single component input vtkImgeData: "
              << numberOfComponents << std::endl;
    return;
    }

  int* dim = data->GetDimensions();

  img.set_size(dim[0], dim[1], numberOfComponents);
  vxl_byte* vxlPtr = img.top_left_ptr();

  // vtk images have their origin at the bottom left, so we flip the y axis
  const unsigned char* vtkptr =
    static_cast<unsigned char*>(data->GetScalarPointer());
  vtkptr += img.ni() * (img.nj() - 1) * numberOfComponents;

  vxl_byte* vxlptr = img.top_left_ptr();
  ptrdiff_t istp = img.istep(), jstp = img.jstep();
  ptrdiff_t pstep = img.planestep();
  ptrdiff_t vtkrstep = numberOfComponents * img.ni();
  for (unsigned int p = 0; p < numberOfComponents;
       p++, vxlptr += pstep, ++vtkptr)
    {
    vxl_byte* vxlpl = vxlptr;
    const unsigned char* vtkch = vtkptr;
    for (unsigned int j = 0; j < img.nj();
         j++, vxlpl += jstp, vtkch -= vtkrstep)
      {
      vxl_byte* vxlrow = vxlpl;
      const unsigned char* vtkrow = vtkch;
      for (unsigned int i = 0; i < img.ni();
           i++, vxlrow += istp, vtkrow += numberOfComponents)
        {
        *vxlrow = *vtkrow;
        }
      }
    }
}
