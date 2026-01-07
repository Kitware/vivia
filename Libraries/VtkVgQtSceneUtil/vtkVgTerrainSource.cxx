// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgTerrainSource.h"

// VisGUI includes
#include "vtkVgCoordinateTransform.h"

#include <qtKstReader.h>
#include <qtStlUtil.h>

#include <vgGeodesy.h>
#include <vgGeoTypes.h>

#include <vtkVgBaseImageSource.h>
#include <vtkVgMultiResJpgImageReader2.h>
#include <vtkVgTerrain.h>
#include <vtkVgUtil.h>

// VTK includes.
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

// Qt includes
#include <QDir>
#include <QFileInfo>

vtkStandardNewMacro(vtkVgTerrainSource);

namespace // anonymous
{

//-----------------------------------------------------------------------------
inline double min(double a, double b, double c, double d)
{
#if __cplusplus >= 201103L
  return std::min({ a, b, c, d });
#else
  return std::min(std::min(a, b), std::min(c, d));
#endif
}

//-----------------------------------------------------------------------------
inline double max(double a, double b, double c, double d)
{
#if __cplusplus >= 201103L
  return std::max({ a, b, c, d });
#else
  return std::max(std::max(a, b), std::max(c, d));
#endif
}

//-----------------------------------------------------------------------------
inline double dist(const vgPoint2d& a, const vgPoint2d b)
{
  const double dx = a.X - b.X;
  const double dy = a.Y - b.Y;
  return sqrt((dx * dx) + (dy * dy));
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
vtkVgTerrainSource::vtkVgTerrainSource() : vtkVgDataSourceBase()
{
  this->BaseTile = NULL;
  this->BaseTileData = NULL;
  this->CoordinateTransformMatrix = NULL;
  this->ImageLevel = 4;
}

//-----------------------------------------------------------------------------
vtkVgTerrainSource::~vtkVgTerrainSource()
{
  // Do nothing
}

//-----------------------------------------------------------------------------
void vtkVgTerrainSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "DataSource: " << (this->DataSource ? "NULL" : this->DataSource) << "\n";
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkVgTerrain> vtkVgTerrainSource::CreateTerrain()
{
  if (!this->DataSource)
    {
    vtkErrorMacro(<< "Invalid data source.\n");
    return 0;
    }

  QUrl uri = QUrl::fromEncoded(this->GetDataSource());
  qtKstReader reader(uri);

  vtkVgTerrain::SmartPtr terrain = vtkVgTerrain::SmartPtr::New();

  // \NOTE: Assuming that base tile is the first tile listed.
  int index = 0;

  if (reader.isValid())
    {
    while (!reader.isEndOfFile())
      {
      QString imageFile;
      double  gsd;
      vgGeocodedTile geoTile;

      if (!reader.readString(imageFile, 0))
        return 0;

      // Get image absolute path.
      // \TODO: handle non-local files?
      QFileInfo fi(uri.toLocalFile());
      QDir sourceDir(fi.canonicalPath());
      QFileInfo ifi(sourceDir, imageFile);
      if (!ifi.exists())
        return 0;

      imageFile = ifi.canonicalFilePath();

      // Ignore for now.
      if (!reader.readReal(gsd, 1))
        return 0;

      // Read bounds.
      if (!reader.readInt(geoTile.GCS, 2))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[0].Longitude, 3))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[0].Latitude, 4))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[1].Longitude, 5))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[1].Latitude, 6))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[3].Longitude, 7))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[3].Latitude, 8))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[2].Longitude, 9))
        return 0;
      if (!reader.readReal(geoTile.Coordinate[2].Latitude, 10))
        return 0;

      // Convert bounds to WGS84 lat/lon, as we generally assume that world
      // coordinates are in this system.
      for (int n = 0; n < 4; ++n)
        {
        const vgGeocodedCoordinate coord(geoTile.Coordinate[n], geoTile.GCS);
        geoTile.Coordinate[n] =
          vgGeodesy::convertGcs(coord, vgGeodesy::LatLon_Wgs84);
        }

      vtkSmartPointer<vtkImageActor> prop;
      std::string ext = vtksys::SystemTools::LowerCase(
                          vtksys::SystemTools::GetFilenameLastExtension(stdString(imageFile)));
      if (ext.compare(".mrj") == 0)
        {
        prop = vtkSmartPointer<vtkImageActor>::New();

        // Base tile.
        if (index == 0)
          {
          this->BaseTile = vtkSmartPointer<vtkVgMultiResJpgImageReader2>::New();
          this->BaseTile->SetFileName(qPrintable(imageFile));
          this->BaseTile->SetLevel(this->ImageLevel);
          this->BaseTile->Update();

          this->BaseTileData = vtkSmartPointer<vtkImageData>::New();
          this->BaseTileData->ShallowCopy(this->BaseTile->GetOutput());

          prop->SetInputData(this->BaseTileData);
          }
        else
          {
          this->OtherTiles.push_back(vtkSmartPointer<vtkVgMultiResJpgImageReader2>::New());
          this->OtherTiles.back()->SetFileName(qPrintable(imageFile));
          this->OtherTiles.back()->SetLevel(this->ImageLevel);
          this->OtherTiles.back()->Update();

          this->OtherTilesData.push_back(vtkSmartPointer<vtkImageData>::New());
          this->OtherTilesData.back()->ShallowCopy(this->OtherTiles.back()->GetOutput());

          prop->SetInputData(this->OtherTilesData.back());
          }
        }
      else
        {
        return 0;
        }

      // Coordinate transform is wrt to base tile only.
      if (index == 0)
        {
        vtkVgCoordinateTransform::SmartPtr coordinateTransform(vtkVgCoordinateTransform::SmartPtr::New());
        coordinateTransform->SetFromPoints(
          geoTile.Coordinate[0].Longitude, geoTile.Coordinate[0].Latitude,
          geoTile.Coordinate[1].Longitude, geoTile.Coordinate[1].Latitude,
          geoTile.Coordinate[2].Longitude, geoTile.Coordinate[2].Latitude,
          geoTile.Coordinate[3].Longitude, geoTile.Coordinate[3].Latitude);

        double bounds[6];
        this->BaseTileData->GetBounds(bounds);
        coordinateTransform->SetToPoints(bounds[0], bounds[3],
                                         bounds[1], bounds[3],
                                         bounds[1], bounds[2],
                                         bounds[0], bounds[2]);

        this->CoordinateTransformMatrix = coordinateTransform->GetHomographyMatrix();
        }
      else
        {
        // Move other tiles into the reference frame of base tile.
        vtkVgCoordinateTransform::SmartPtr coordinateTransform(vtkVgCoordinateTransform::SmartPtr::New());
        coordinateTransform->SetToPoints(
          geoTile.Coordinate[0].Longitude, geoTile.Coordinate[0].Latitude,
          geoTile.Coordinate[1].Longitude, geoTile.Coordinate[1].Latitude,
          geoTile.Coordinate[2].Longitude, geoTile.Coordinate[2].Latitude,
          geoTile.Coordinate[3].Longitude, geoTile.Coordinate[3].Latitude);

        double bounds[6];
        this->OtherTilesData.back()->GetBounds(bounds);
        coordinateTransform->SetFromPoints(bounds[0], bounds[3],
                                           bounds[1], bounds[3],
                                           bounds[1], bounds[2],
                                           bounds[0], bounds[2]);

        vtkSmartPointer<vtkMatrix4x4> imageToLatLonMatrix =
          coordinateTransform->GetHomographyMatrix();
        vtkSmartPointer<vtkMatrix4x4> transformMatrix =
          vtkSmartPointer<vtkMatrix4x4>::New();

        vtkMatrix4x4::Multiply4x4(this->CoordinateTransformMatrix,
                                  imageToLatLonMatrix, transformMatrix);

        vtkSmartPointer<vtkMatrix4x4> newMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        newMatrix->DeepCopy(transformMatrix);

        if (prop)
          {
          double w = transformMatrix->GetElement(3, 3);
          for (int i = 0; i < 4; ++i)
            {
            for (int j = 0; j < 4; ++j)
              {
              transformMatrix->SetElement(i, j, transformMatrix->GetElement(i, j) / w);
              }
            }
          transformMatrix->SetElement(2, 2, 1.0);
          prop->SetUserMatrix(transformMatrix);
          newMatrix->Invert();
          this->OtherTileTileToBaseMatrices.push_back(transformMatrix);
          this->OtherTileBaseToTileMatrices.push_back(newMatrix);
          }
        }

      if (prop)
        {
        terrain->AddDrawable(prop);
        }
      else
        {
        return 0;
        }

      ++index;
      reader.nextRecord();
      }

    return terrain;
    }
  else
    {
    return 0;
    }
}

//-----------------------------------------------------------------------------
void vtkVgTerrainSource::SetImageLevel(const int level)
{
  if (this->ImageLevel != level)
    {
    this->ImageLevel = level;
    this->Modified();
    }
  if (this->BaseTile)
    {
    this->BaseTile->SetLevel(level);
    }

  size_t numberOfOtherTiles = this->OtherTiles.size();
  for (size_t i = 0; i < numberOfOtherTiles; ++i)
    {
    this->OtherTiles[i]->SetLevel(level);
    }
}

//-----------------------------------------------------------------------------
void vtkVgTerrainSource::SetVisibleScale(const double& scale)
{
  if (this->BaseTile)
    {
    this->BaseTile->SetScale(scale);
    }

  size_t numberOfOtherTiles = this->OtherTiles.size();
  for (size_t i = 0; i < numberOfOtherTiles; ++i)
    {
    vtkVgBaseImageSource* tile = this->OtherTiles[i];

    int dim[2];
    tile->GetDimensions(dim);

    // Get the transformation matrix for the tile
    vtkMatrix4x4* mat = this->OtherTileTileToBaseMatrices[i];

    // Get the transformed the tile corners
    const vgPoint2d pt1 = vtkVgApplyHomography(0,      0,      mat);
    const vgPoint2d pt2 = vtkVgApplyHomography(0,      dim[1], mat);
    const vgPoint2d pt3 = vtkVgApplyHomography(dim[0], dim[1], mat);
    const vgPoint2d pt4 = vtkVgApplyHomography(dim[0], 0,      mat);

    // Compute the relative scale of each edge of the tile
    // TODO: This ignores the fact that, depending on how the tile is warped,
    //       the ideal scale may not be uniform across the entire tile... for
    //       now, we're just using the highest LOD required
    const double k1 = dim[1] / dist(pt1, pt2);
    const double k2 = dim[0] / dist(pt2, pt3);
    const double k3 = dim[1] / dist(pt3, pt4);
    const double k4 = dim[0] / dist(pt4, pt1);

    // Set the tile scale to the reference scale adjusted by the smallest
    // relative scale
    tile->SetScale(scale * min(k1, k2, k3, k4));
    }
}

//-----------------------------------------------------------------------------
void vtkVgTerrainSource::SetVisibleExtents(const int extents[4])
{
  if (this->BaseTile)
    {
    this->BaseTile->SetReadExtents(const_cast<int*>(extents));
    }

  size_t numberOfOtherTiles = this->OtherTiles.size();
  for (size_t i = 0; i < numberOfOtherTiles; ++i)
    {
    const vtkMatrix4x4* const mat = this->OtherTileBaseToTileMatrices[i];
    const vgPoint2d pt1 = vtkVgApplyHomography(extents[0], extents[2], mat);
    const vgPoint2d pt2 = vtkVgApplyHomography(extents[0], extents[3], mat);
    const vgPoint2d pt3 = vtkVgApplyHomography(extents[1], extents[2], mat);
    const vgPoint2d pt4 = vtkVgApplyHomography(extents[1], extents[3], mat);

    int newExtents[6] =
      {
      static_cast<int>(floor(min(pt1.X, pt2.X, pt3.X, pt4.X))),
      static_cast<int>( ceil(max(pt1.X, pt2.X, pt3.X, pt4.X))),
      static_cast<int>(floor(min(pt1.Y, pt2.Y, pt3.Y, pt4.Y))),
      static_cast<int>( ceil(max(pt1.Y, pt2.Y, pt3.Y, pt4.Y))),
      0, 0
      };

    this->OtherTiles[i]->SetReadExtents(const_cast<int*>(newExtents));
    }
}

//-----------------------------------------------------------------------------
vtkMatrix4x4* vtkVgTerrainSource::GetCoordinateTransformMatrix() const
{
  return this->CoordinateTransformMatrix;
}

//-----------------------------------------------------------------------------
void vtkVgTerrainSource::Update()
{
  if (this->BaseTile)
    {
    this->BaseTile->Update();

    this->BaseTileData->ShallowCopy(this->BaseTile->GetOutput());
    }

  size_t numberOfOtherTiles = this->OtherTiles.size();
  for (size_t i = 0; i < numberOfOtherTiles; ++i)
    {
    this->OtherTiles[i]->Update();
    this->OtherTilesData[i]->ShallowCopy(this->OtherTiles[i]->GetOutput());
    }
}
