/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpProjectParser.h"

// Application includes.
#include "vpProject.h"

// qtExtensions includes.
#include <qtStlUtil.h>

// VIDTK includes.
#ifdef VISGUI_USE_VIDTK
#include <utilities/config_block.h>
#include <utilities/config_block_parser.h>
#include <utilities/token_expansion.h>
#endif

// VTK includes.
#include <vtkMatrix4x4.h>
#include <vtksys/SystemTools.hxx>

// Qt includes.
#include <QDebug>

// C++ includes.
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
  blk.add_parameter(prj->TrackPVOsFileTag, "", "Filename containing PVO data for the tracks");
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

  if (this->UseStream)
    {
    iss.str(this->ProjectStream);
    blkParser.set_input_stream(iss);
    }
  else
    {
    blkParser.set_filename(this->ProjectFileName);
    prj->ConfigFileStem =
      this->ProjectFileName.substr(0, this->ProjectFileName.find_last_of("/\\"));
    prj->ConfigFileStem = prj->ConfigFileStem.append("/");
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
  vpProject::TagFileMapItr bItr = prj->TagFileMap.begin();
  while (bItr != prj->TagFileMap.end())
    {
    *bItr->second = blk.get<std::string>(bItr->first);

    *bItr->second = vidtk::token_expansion::expand_token(*bItr->second);

    this->ConstructCompletePath(*bItr->second, prj->ConfigFileStem);

    if (bItr->first == prj->DataSetSpecifierTag ||
        bItr->first == prj->TracksFileTag)
      {
      prj->SetIsValid(*bItr->second, (*bItr->second).empty() ?
                      vpProject::FILE_NAME_EMPTY : vpProject::FILE_NAME_NOT_EMPTY);
      }
    else
      {
      prj->SetIsValid(*bItr->second,
                      this->CheckIfFileExists(bItr->first, *bItr->second));
      }

    ++bItr;
    }

  prj->PrecomputeActivity = blk.get<int>(prj->PrecomputeActivityTag);
  prj->OverviewSpacing = blk.get<double>(prj->OverviewSpacingTag);
  prj->ColorWindow = blk.get<double>(prj->ColorWindowTag);
  prj->ColorLevel = blk.get<double>(prj->ColorLevelTag);
  prj->ColorMultiplier = blk.get<double>(prj->ColorMultiplierTag);
  prj->FrameNumberOffset = blk.get<double>(prj->FrameNumberOffsetTag);
  blk.get(prj->OverviewOriginTag,      prj->OverviewOrigin);
  blk.get(prj->AnalysisDimensionsTag,  prj->AnalysisDimensions);
  blk.get(prj->AOIUpperLeftLatLonTag,  prj->AOIUpperLeftLatLon);
  blk.get(prj->AOIUpperRightLatLonTag, prj->AOIUpperRightLatLon);
  blk.get(prj->AOILowerLeftLatLonTag, prj->AOILowerLeftLatLon);
  blk.get(prj->AOILowerRightLatLonTag, prj->AOILowerRightLatLon);

  if (blk.get(prj->TrackColorOverrideTag, prj->TrackColorOverride))
    {
    prj->HasTrackColorOverride = true;
    }

  if (blk.get(prj->ImageToGcsMatrixTag,  prj->ImageToGcsMatrixArray))
    {
    prj->ImageToGcsMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

    // Get the number of rows and columns
    int noOfRows = prj->ImageToGcsMatrixArray[0];
    int noOfCols = prj->ImageToGcsMatrixArray[1];

    // 3x3
    if (noOfRows == 3 && noOfCols == 3)
      {
      prj->ImageToGcsMatrix->SetElement(0, 0, prj->ImageToGcsMatrixArray[2]);
      prj->ImageToGcsMatrix->SetElement(0, 1, prj->ImageToGcsMatrixArray[3]);
      prj->ImageToGcsMatrix->SetElement(0, 3, prj->ImageToGcsMatrixArray[4]);

      prj->ImageToGcsMatrix->SetElement(1, 0, prj->ImageToGcsMatrixArray[5]);
      prj->ImageToGcsMatrix->SetElement(1, 1, prj->ImageToGcsMatrixArray[6]);
      prj->ImageToGcsMatrix->SetElement(1, 3, prj->ImageToGcsMatrixArray[7]);

      prj->ImageToGcsMatrix->SetElement(3, 0, prj->ImageToGcsMatrixArray[8]);
      prj->ImageToGcsMatrix->SetElement(3, 1, prj->ImageToGcsMatrixArray[9]);
      prj->ImageToGcsMatrix->SetElement(3, 3, prj->ImageToGcsMatrixArray[10]);
      }
    // 3X4
    else if (noOfRows == 3 && noOfCols == 4)
      {
      prj->ImageToGcsMatrix->SetElement(0, 0, prj->ImageToGcsMatrixArray[2]);
      prj->ImageToGcsMatrix->SetElement(0, 1, prj->ImageToGcsMatrixArray[3]);
      prj->ImageToGcsMatrix->SetElement(0, 2, prj->ImageToGcsMatrixArray[4]);
      prj->ImageToGcsMatrix->SetElement(0, 3, prj->ImageToGcsMatrixArray[5]);

      prj->ImageToGcsMatrix->SetElement(1, 0, prj->ImageToGcsMatrixArray[6]);
      prj->ImageToGcsMatrix->SetElement(1, 1, prj->ImageToGcsMatrixArray[7]);
      prj->ImageToGcsMatrix->SetElement(1, 2, prj->ImageToGcsMatrixArray[8]);
      prj->ImageToGcsMatrix->SetElement(1, 3, prj->ImageToGcsMatrixArray[9]);

      prj->ImageToGcsMatrix->SetElement(3, 0, prj->ImageToGcsMatrixArray[10]);
      prj->ImageToGcsMatrix->SetElement(3, 1, prj->ImageToGcsMatrixArray[11]);
      prj->ImageToGcsMatrix->SetElement(3, 2, prj->ImageToGcsMatrixArray[12]);
      prj->ImageToGcsMatrix->SetElement(3, 3, prj->ImageToGcsMatrixArray[13]);
      }
    //4x3
    else if (noOfRows == 4 && noOfCols == 3)
      {
      prj->ImageToGcsMatrix->SetElement(0, 0, prj->ImageToGcsMatrixArray[2]);
      prj->ImageToGcsMatrix->SetElement(0, 1, prj->ImageToGcsMatrixArray[3]);
      prj->ImageToGcsMatrix->SetElement(0, 3, prj->ImageToGcsMatrixArray[4]);

      prj->ImageToGcsMatrix->SetElement(1, 0, prj->ImageToGcsMatrixArray[5]);
      prj->ImageToGcsMatrix->SetElement(1, 1, prj->ImageToGcsMatrixArray[6]);
      prj->ImageToGcsMatrix->SetElement(1, 3, prj->ImageToGcsMatrixArray[7]);

      prj->ImageToGcsMatrix->SetElement(2, 0, prj->ImageToGcsMatrixArray[8]);
      prj->ImageToGcsMatrix->SetElement(2, 1, prj->ImageToGcsMatrixArray[9]);
      prj->ImageToGcsMatrix->SetElement(2, 3, prj->ImageToGcsMatrixArray[10]);

      prj->ImageToGcsMatrix->SetElement(3, 0, prj->ImageToGcsMatrixArray[11]);
      prj->ImageToGcsMatrix->SetElement(3, 1, prj->ImageToGcsMatrixArray[12]);
      prj->ImageToGcsMatrix->SetElement(3, 3, prj->ImageToGcsMatrixArray[13]);
      }
    else if(noOfRows == 4 && noOfCols ==4)
      {
        prj->ImageToGcsMatrix->DeepCopy(&prj->ImageToGcsMatrixArray[2]);
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
void vpProjectParser::ConstructCompletePath(std::string& file, const std::string& stem)
{
  // First remove carriage return.
  std::string::size_type pos = std::string::npos;

  pos = file.rfind('\r', pos);

  if (pos != std::string::npos)
    {
    file.erase(pos, 1);
    }

  if (!file.empty() &&
      !vtksys::SystemTools::FileIsFullPath(file.c_str()))
    {
    file = stem + file;
    }
}

//-----------------------------------------------------------------------------
int vpProjectParser::CheckIfFileExists(const std::string& tag, const std::string& fileName)
{
  if (fileName.empty())
    {
    return vpProject::FILE_NAME_EMPTY;
    }

  if (!vtksys::SystemTools::FileExists(fileName.c_str(), true))
    {
    qWarning() << "WARNING:" << tag.c_str() << '(' << qtString(fileName) << ')'
               << "does not exist; application may not work properly";

    return vpProject::FILE_NOT_EXIST;
    }

  return vpProject::FILE_EXIST;
}
