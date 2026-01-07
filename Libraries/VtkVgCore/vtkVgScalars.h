// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgScalars_h
#define __vtkVgScalars_h

#include <vtkObject.h>

#include <map>

#include <vgExport.h>

class vtkVgTimeStamp;

class VTKVG_CORE_EXPORT vtkVgScalars : public vtkObject
{
public:
  vtkTypeMacro(vtkVgScalars, vtkObject);

  static vtkVgScalars* New();

  // Description:
  // Define a value that will be identified as the value
  // that will be returned when no scalars is found
  // at a given timestamp.
  static void SetNotFoundValue(double value);
  static double GetNotFoundValue();

  virtual void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Check if there is a scalar data at the given timestamp
  bool HasValue(const vtkVgTimeStamp& timestamp) const;

  // Description:
  // Insert a new value at time t = \c timestamp
  void InsertValue(const vtkVgTimeStamp& timestamp, const double& value);

  // Description:
  // Insert multiple values
  void InsertValues(const std::map<vtkVgTimeStamp, double>& values);

  // Description:
  // Get scalar data at the given timestamp. If interpolate is
  // true, then try to compute a new value by performing linear interpolation
  // in case the data is not available at t = \c timestamp.
  double GetValue(const vtkVgTimeStamp& timestamp,
                  bool interpolate = false) const;

  // Description:
  // Get current range of scalars
  const double* GetRange();

protected:
  vtkVgScalars();
  virtual ~vtkVgScalars();

  static double NotFoundValue;

  class vtkVgScalarsInternal;
  vtkVgScalarsInternal* Implementation;

private:
  vtkVgScalars(const vtkVgScalars&); // Not implemented
  vtkVgScalars& operator=(const vtkVgScalars&); // Not implemented
};

#endif // __vtkVgScalars_h
