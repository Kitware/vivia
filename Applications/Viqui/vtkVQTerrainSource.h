/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVQTerrainSource_h
#define __vtkVQTerrainSource_h

// VG includes.
#include "vtkVgMacros.h"
#include "vtkVgDataSourceBase.h"

// VTK includes.
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

// STL includes.
#include <vector>

// Forward declarations.
class vtkProp;
class vtkImageData;
class vtkVgTerrain;
class vtkVgBaseImageSource;

class vtkVQTerrainSource : public vtkVgDataSourceBase
{
public:
  vtkVgClassMacro(vtkVQTerrainSource);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVQTerrainSource, vtkVgDataSourceBase);

  static vtkVQTerrainSource* New();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Given a data source generate a terrain.
  vtkSmartPointer<vtkVgTerrain> CreateTerrain();

  // Description:
  // Overridden functions.
  virtual void SetVisibleExtents(const int extents[4]);
  virtual void SetVisibleScale(const double& scale);

  // Description:
  // Get coodinate system transformation. Currently this will transform
  // points in lat / lon space to image space.
  vtkMatrix4x4* GetCoordinateTransformMatrix() const;

  // Description:
  // Update data produced by this source.
  virtual void Update();

  // Description:
  // Control the LOD that will be read from the context.  Use -1 to autocompute.
  // autocompute.  Defaults to 4 (should do for initial read, because extents
  // and scale not yet set).
  virtual void SetImageLevel(int level);
  vtkGetMacro(ImageLevel, int);

protected:
  vtkVQTerrainSource();
  virtual ~vtkVQTerrainSource();

  vtkSmartPointer<vtkVgBaseImageSource> BaseTile;
  std::vector<vtkSmartPointer<vtkVgBaseImageSource> > OtherTiles;

  vtkSmartPointer<vtkImageData> BaseTileData;
  std::vector<vtkSmartPointer<vtkImageData> > OtherTilesData;

  vtkSmartPointer<vtkMatrix4x4> CoordinateTransformMatrix;
  std::vector<vtkSmartPointer<vtkMatrix4x4> > OtherTileBaseToTileMatrices;
  std::vector<vtkSmartPointer<vtkMatrix4x4> > OtherTileTileToBaseMatrices;
  int ImageLevel;

private:
  vtkVQTerrainSource(const vtkVQTerrainSource&);
  void operator= (const vtkVQTerrainSource&);
};

#endif // __vtkVQTerrainSource_h
