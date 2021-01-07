// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vtkVgTerrainSource_h
#define __vtkVgTerrainSource_h

// VG includes.
#include <vgExport.h>

#include <vtkVgMacros.h>
#include <vtkVgDataSourceBase.h>

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

class VTKVGQT_SCENEUTIL_EXPORT vtkVgTerrainSource : public vtkVgDataSourceBase
{
public:
  vtkVgClassMacro(vtkVgTerrainSource);

  // Description:
  // Usual VTK functions.
  vtkTypeMacro(vtkVgTerrainSource, vtkVgDataSourceBase);

  static vtkVgTerrainSource* New();

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
  vtkVgTerrainSource();
  virtual ~vtkVgTerrainSource();

  vtkSmartPointer<vtkVgBaseImageSource> BaseTile;
  std::vector<vtkSmartPointer<vtkVgBaseImageSource> > OtherTiles;

  vtkSmartPointer<vtkImageData> BaseTileData;
  std::vector<vtkSmartPointer<vtkImageData> > OtherTilesData;

  vtkSmartPointer<vtkMatrix4x4> CoordinateTransformMatrix;
  std::vector<vtkSmartPointer<vtkMatrix4x4> > OtherTileBaseToTileMatrices;
  std::vector<vtkSmartPointer<vtkMatrix4x4> > OtherTileTileToBaseMatrices;
  int ImageLevel;

private:
  vtkVgTerrainSource(const vtkVgTerrainSource&);
  void operator= (const vtkVgTerrainSource&);
};

#endif // __vtkVgTerrainSource_h
