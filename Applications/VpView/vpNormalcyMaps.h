// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
