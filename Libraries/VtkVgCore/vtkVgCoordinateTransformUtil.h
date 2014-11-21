#ifndef VTKVGCOORDINATETRANSFORMUTIL_H
#define VTKVGCOORDINATETRANSFORMUTIL_H

#include <vgExport.h>

// VTK includes
#include <vtkSmartPointer.h>

// Forward declarations
class vtkMatrix4x4;

class VTKVG_CORE_EXPORT vtkVgCoordinateTransformUtil
{
public:
   static vtkSmartPointer<vtkMatrix4x4>
    ComputeImageToLatLonMatrix(double llx, double lly,
                               double urx, double ury,
                               double geollx, double geolly,
                               double geolrx, double geolry,
                               double geourx, double geoury,
                               double geoulx, double geouly);
};

#endif // VTKVGCOORDINATETRANSFORMUTIL_H
