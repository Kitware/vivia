/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

// Qt includes.
#include <QString>

// VTK includes.
#include <vtkSmartPointer.h>

// Forward declarations.
class vtkActor;
class vtkImageData;

class vpNormalcyMaps
{
public:

  vpNormalcyMaps();
  ~vpNormalcyMaps();

  int  LoadFromFile(const QString& filename);

  void SetPrecompute(int state = 0);
  int  GetPrecompute() const;

  QString GetNormalcySource(const QString& key) const;

  vtkSmartPointer<vtkImageData> GetNormalcyImage(const QString& key) const;

  void GetAllNormalcyKeys(QList<QString>& keys) const;

  QList<vtkSmartPointer<vtkActor> > GetAllNormalcyActors() const;

private:
  class vpNormalcyMapsInternal;
  vpNormalcyMapsInternal* Internal;

  vpNormalcyMaps(const vpNormalcyMaps& src);  // Not implemented.
  void operator=(const vpNormalcyMaps& src);  // Not implemented.
};
