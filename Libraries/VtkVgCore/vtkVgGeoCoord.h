// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgGeoCoord_h
#define __vtkVgGeoCoord_h

#include <vgExport.h>

#include <vgGeodesy.h>
#include <vgGeoTypes.h>

#include "vtkVgPythonUtil.h"

class VTKVG_CORE_EXPORT vtkVgGeoCoord : public vgGeocodedCoordinate
{
public:
  vtkVgGeoCoord() {}
  vtkVgGeoCoord(double northing, double easting,
                int gcs = vgGeodesy::LatLon_Wgs84)
    {
    this->Northing = northing;
    this->Easting = easting;
    this->GCS = gcs;
    }

  vtkVgGeoCoord(const vgGeocodedCoordinate& other) :
    vgGeocodedCoordinate(other)
    {}

  vtkVgPythonGetSet(int, GCS);
  vtkVgPythonGetSet(double, Northing);
  vtkVgPythonGetSet(double, Easting);
  vtkVgPythonGetSet(double, Latitude);
  vtkVgPythonGetSet(double, Longitude);

  bool IsValid() const { return this->GCS != -1; }
};

#endif
