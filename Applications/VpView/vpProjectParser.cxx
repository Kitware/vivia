/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpProjectParser.h"

#include "vgGeodesy.h"

#include <qtEnumerate.h>
#include <qtStlUtil.h>

#ifdef VISGUI_USE_VIDTK
#include <utilities/config_block.h>
#include <utilities/config_block_parser.h>
#include <utilities/token_expansion.h>
#endif

#include <vtkMatrix4x4.h>
#include <vtksys/SystemTools.hxx>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <exception>
#include <iomanip>
#include <sstream>

//-----------------------------------------------------------------------------
vpProjectParser::vpProjectParser() :
  UseStream(false),
  ProjectFileName(""),
  ProjectStream("")
{
}

//-----------------------------------------------------------------------------
vpProjectParser::~vpProjectParser()
{
}

//-----------------------------------------------------------------------------
bool vpProjectParser::Parse(vpProject* prj)
{
  if (!prj)
    {
    qWarning() << "Invalid or NULL project";
    return false;
    }

#ifdef VISGUI_USE_VIDTK
  vidtk::config_block blk;
  blk.add_parameter(prj->OverviewFileTag, "", "Image for showing context in & around analysis boundary");
  blk.add_parameter(prj->DataSetSpecifierTag, "", "Filename with list of images for each frame or glob for sequence of images");
  blk.add_parameter(prj->TracksFileTag, "", "Filename or glob containing the tracks data");
  blk.add_parameter(prj->TrackTraitsFileTag, "", "Filename containing extra track data (normalcy, etc.)");
  blk.add_parameter(prj->EventsFileTag, "", "Filename containing the events data");
  blk.add_parameter(prj->EventLinksFileTag, "", "Filename containing the event linking data");
  blk.add_parameter(prj->ActivitiesFileTag, "", "Filename containing the activities data");
  blk.add_parameter(prj->IconsFileTag, "", "Filename containg the icons sheet and parameters");
  blk.add_parameter(prj->InformaticsIconFileTag, "", "Image filename containing icons for the informatics");
  blk.add_parameter(prj->NormalcyMapsFileTag, "", "Filename containing normalcy map (key, filename)");
  blk.add_parameter(prj->PrecomputeActivityTag, "0", "Precompute activity at load time");
  blk.add_parameter(prj->OverviewSpacingTag, "1.0", "Specify spacing (single parameter) for the overview image");
  blk.add_parameter(prj->OverviewOriginTag, "0.0 0.0", "Specify origin (x y) for the overview image");
  blk.add_parameter(prj->AnalysisDimensionsTag, "-1.0 -1.0", "Specify analysis dimensions (width height) for the overview image");
  blk.add_parameter(prj->AOIUpperLeftLatLonTag, "444 444", "Specify upper-left corner of AOI (lat/lon)");
  blk.add_parameter(prj->AOIUpperRightLatLonTag, "444 444", "Specify upper-right corner of AOI (lat/lon)");
  blk.add_parameter(prj->AOILowerLeftLatLonTag, "444 444", "Specify lower-left corner of AOI (lat/lon)");
  blk.add_parameter(prj->AOILowerRightLatLonTag, "444 444", "Specify lower-right corner of AOI (lat/lon)");
  blk.add_parameter(prj->TrackColorOverrideTag, "Force a track color for this project (r g b)");
  blk.add_parameter(prj->ColorWindowTag, "255.0", "Specify color value range (window) for data set");
  blk.add_parameter(prj->ColorLevelTag, "127.5", "Specify color value mean (level) for data set");
  blk.add_parameter(prj->ColorMultiplierTag, "1.0", "Specify color multiplier for objects");
  blk.add_parameter(prj->FrameNumberOffsetTag, "0", "Frame number offset of imagery data");
  blk.add_parameter(prj->ImageTimeMapFileTag, "", "Filename containing image timestamps");
  blk.add_parameter(prj->HomographyIndexFileTag, "", "Filename containing image homographies");
  blk.add_parameter(prj->FiltersFileTag, "", "Filename containing filters to import");
  blk.add_parameter(prj->ImageToGcsMatrixTag, "", "Image to geographic coordinate matrix");
  blk.add_parameter(prj->SceneElementsFileTag, "", "Filename containing scene element data");

  vidtk::config_block_parser blkParser;
  std::istringstream iss;

  QDir projectDir;
  if (this->UseStream)
    {
    iss.str(stdString(this->ProjectStream));
    blkParser.set_input_stream(iss);
    }
  else
    {
    blkParser.set_filename(stdString(this->ProjectFileName));
    projectDir = QDir{QFileInfo{this->ProjectFileName}.canonicalPath()};
    }

  // \note: Exception is thrown by the parser if a unknown tag is found
  // in the project.
  try
    {
    blk.set_fail_on_missing_block(false);
    blkParser.parse(blk);
    }
  catch (std::exception& e)
    {
    qWarning() << "ERROR:" << e.what();
    }
  catch (...)
    {
    qWarning() << "ERROR: Unknown error occurred during project parsing";
    }

  // Various file sources used for the data.
  foreach (const auto& fileTag, qtEnumerate(prj->TagFileMap))
    {
    const auto& tag = fileTag.key();
    auto& filePath = *(fileTag.value());

    const auto& value = blk.get<std::string>(stdString(tag));
    filePath = qtString(vidtk::token_expansion::expand_token(value));
    if (!filePath.isEmpty())
      {
      filePath = projectDir.absoluteFilePath(filePath);
      }

    if (tag == prj->DataSetSpecifierTag ||
        tag == prj->TracksFileTag)
      {
      const auto expectedState =
        (filePath.isEmpty() ? vpProject::FILE_NAME_EMPTY
                            : vpProject::FILE_NAME_NOT_EMPTY);
      prj->SetIsValid(filePath, expectedState);
      }
    else
      {
      const auto expectedState =
        this->CheckIfFileExists(tag, filePath);
      prj->SetIsValid(filePath, expectedState);
      }
    }

  // Read non-string fields that can be read directly
  prj->PrecomputeActivity = blk.get<int>(prj->PrecomputeActivityTag);
  prj->OverviewSpacing = blk.get<double>(prj->OverviewSpacingTag);
  prj->ColorWindow = blk.get<double>(prj->ColorWindowTag);
  prj->ColorLevel = blk.get<double>(prj->ColorLevelTag);
  prj->ColorMultiplier = blk.get<double>(prj->ColorMultiplierTag);
  prj->FrameNumberOffset = blk.get<double>(prj->FrameNumberOffsetTag);

  // Read various fields that consist of two numbers
  double pt[2];

  blk.get(prj->OverviewOriginTag, pt);
  prj->OverviewOrigin = {pt[0], pt[1]};

  blk.get(prj->AnalysisDimensionsTag, pt);
  prj->AnalysisDimensions = {pt[0], pt[1]};

  prj->AOI.GCS = vgGeodesy::LatLon_Wgs84;
  const char* aoiTags[4] = {
    prj->AOIUpperLeftLatLonTag,
    prj->AOIUpperRightLatLonTag,
    prj->AOILowerRightLatLonTag,
    prj->AOILowerLeftLatLonTag};
  for (int i = 0; i < 4; ++i)
    {
    blk.get(aoiTags[i], pt);
    if (fabs(pt[0]) > 360.0 || fabs(pt[1]) > 360.0)
      {
      prj->AOI.GCS = -1;
      break;
      }
    prj->AOI.Coordinate[i] = {pt[0], pt[1]};
    }

  // Read track override color
  double oc[3];
  if (blk.get(prj->TrackColorOverrideTag, oc))
    {
    prj->TrackColorOverride = vgColor{oc[0], oc[1], oc[2]};
    }

  // Read image-to-GCS matrix
  double i2gMatrix[18];
  if (blk.get(prj->ImageToGcsMatrixTag,  i2gMatrix))
    {
    prj->ImageToGcsMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

    // Get the number of rows and columns
    int noOfRows = i2gMatrix[0];
    int noOfCols = i2gMatrix[1];

    // 3x3
    if (noOfRows == 3 && noOfCols == 3)
      {
      prj->ImageToGcsMatrix->SetElement(0, 0, i2gMatrix[2]);
      prj->ImageToGcsMatrix->SetElement(0, 1, i2gMatrix[3]);
      prj->ImageToGcsMatrix->SetElement(0, 3, i2gMatrix[4]);

      prj->ImageToGcsMatrix->SetElement(1, 0, i2gMatrix[5]);
      prj->ImageToGcsMatrix->SetElement(1, 1, i2gMatrix[6]);
      prj->ImageToGcsMatrix->SetElement(1, 3, i2gMatrix[7]);

      prj->ImageToGcsMatrix->SetElement(3, 0, i2gMatrix[8]);
      prj->ImageToGcsMatrix->SetElement(3, 1, i2gMatrix[9]);
      prj->ImageToGcsMatrix->SetElement(3, 3, i2gMatrix[10]);
      }
    // 3X4
    else if (noOfRows == 3 && noOfCols == 4)
      {
      prj->ImageToGcsMatrix->SetElement(0, 0, i2gMatrix[2]);
      prj->ImageToGcsMatrix->SetElement(0, 1, i2gMatrix[3]);
      prj->ImageToGcsMatrix->SetElement(0, 2, i2gMatrix[4]);
      prj->ImageToGcsMatrix->SetElement(0, 3, i2gMatrix[5]);

      prj->ImageToGcsMatrix->SetElement(1, 0, i2gMatrix[6]);
      prj->ImageToGcsMatrix->SetElement(1, 1, i2gMatrix[7]);
      prj->ImageToGcsMatrix->SetElement(1, 2, i2gMatrix[8]);
      prj->ImageToGcsMatrix->SetElement(1, 3, i2gMatrix[9]);

      prj->ImageToGcsMatrix->SetElement(3, 0, i2gMatrix[10]);
      prj->ImageToGcsMatrix->SetElement(3, 1, i2gMatrix[11]);
      prj->ImageToGcsMatrix->SetElement(3, 2, i2gMatrix[12]);
      prj->ImageToGcsMatrix->SetElement(3, 3, i2gMatrix[13]);
      }
    //4x3
    else if (noOfRows == 4 && noOfCols == 3)
      {
      prj->ImageToGcsMatrix->SetElement(0, 0, i2gMatrix[2]);
      prj->ImageToGcsMatrix->SetElement(0, 1, i2gMatrix[3]);
      prj->ImageToGcsMatrix->SetElement(0, 3, i2gMatrix[4]);

      prj->ImageToGcsMatrix->SetElement(1, 0, i2gMatrix[5]);
      prj->ImageToGcsMatrix->SetElement(1, 1, i2gMatrix[6]);
      prj->ImageToGcsMatrix->SetElement(1, 3, i2gMatrix[7]);

      prj->ImageToGcsMatrix->SetElement(2, 0, i2gMatrix[8]);
      prj->ImageToGcsMatrix->SetElement(2, 1, i2gMatrix[9]);
      prj->ImageToGcsMatrix->SetElement(2, 3, i2gMatrix[10]);

      prj->ImageToGcsMatrix->SetElement(3, 0, i2gMatrix[11]);
      prj->ImageToGcsMatrix->SetElement(3, 1, i2gMatrix[12]);
      prj->ImageToGcsMatrix->SetElement(3, 3, i2gMatrix[13]);
      }
    else if(noOfRows == 4 && noOfCols ==4)
      {
        prj->ImageToGcsMatrix->DeepCopy(&i2gMatrix[2]);
      }
    else
      {
      qWarning() << "ImageToGcsMatrix: matrices with" << noOfRows << "rows and"
                 << noOfCols << "columns are not supported";
      }
    }

  return true;
#else // VISGUI_USE_VIDTK
  // TODO: Write a non-vidtk based project parser.
  return false;
#endif
}

//-----------------------------------------------------------------------------
vpProject::FileState vpProjectParser::CheckIfFileExists(
  const QString& tag, const QString& fileName)
{
  if (fileName.isEmpty())
    {
    return vpProject::FILE_NAME_EMPTY;
    }

  if (!QFileInfo{fileName}.exists())
    {
    qWarning() << "WARNING:" << tag << '(' << fileName << ')'
               << "does not exist; application may not work properly";

    return vpProject::FILE_NOT_EXIST;
    }

  return vpProject::FILE_EXIST;
}
