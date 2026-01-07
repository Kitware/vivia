// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// VTK includes.
#include <vtksys/CommandLineArguments.hxx>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>
#include <vtksys/Glob.hxx>
#include <vtksys/Process.h>

#include <vtkImageShrink3D.h>
#include <vtkImageReader2.h>
#include <vtkImageWriter.h>
#include <vtkImageData.h>
#include <vtkVgMultiResJpgImageWriter2.h>
#include <vtkPNGReader.h>
#include <vtkSmartPointer.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>
#include <vtkTIFFReader.h>
#include <vtkTimerLog.h>
#include <vtkVgImageSource.h>

// STL includes.
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <string>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

//----------------------------------------------------------------------------
void ConvertToMultiResJpg(const char* inFile, const char* outFile,
                          int compressionQuality)
{
  vtkSmartPointer<vtkVgMultiResJpgImageWriter2>
  writer = vtkSmartPointer<vtkVgMultiResJpgImageWriter2>::New();
  writer->SetFileName(outFile);

  if (compressionQuality > 0)
    {
    writer->SetCompressionQuality(compressionQuality);
    }

  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(inFile);

  if (strcasecmp(ext.c_str(), ".png") == 0)
    {
    vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
    reader->SetFileName(inFile);
    reader->Update();
    writer->SetInputData(reader->GetOutputDataObject(0));
    }
  else if (strcasecmp(ext.c_str(), ".tif") == 0 ||
           strcasecmp(ext.c_str(), ".tiff") == 0)
    {
    vtkSmartPointer<vtkTIFFReader> reader =
      vtkSmartPointer<vtkTIFFReader>::New();
    reader->SetFileName(inFile);
    writer->SetInputConnection(reader->GetOutputPort());
    }
  else if (strcasecmp(ext.c_str(), ".jp2") == 0)
    {
    vtkSmartPointer<vtkVgImageSource> reader =
      vtkSmartPointer<vtkVgImageSource>::New();
    reader->SetFileName(inFile);
    reader->Update();
    writer->SetInputData(reader->GetOutputDataObject(0));
    }
  else
    {
    std::cerr << "Files of type " << ext << " are not handled as of now."
              << std::endl;
    std::exit(1);
    }

  writer->Write();
}

int main(int argc, char** argv)
{
  vtksys::CommandLineArguments args;
  args.Initialize(argc, argv);
  args.StoreUnusedArguments(1);

  int quality = 75;
  std::string outDir;

  args.AddArgument("-q", vtksys::CommandLineArguments::SPACE_ARGUMENT,
                   &quality, "Quality (0-100; default 75)");
  args.AddArgument("-o", vtksys::CommandLineArguments::SPACE_ARGUMENT,
                   &outDir, "Output directory");

  if (!args.Parse() || (args.GetUnusedArguments(&argc, &argv), argc < 2))
    {
    cout << "Usage: " << vtksys::SystemTools::GetFilenameName(args.GetArgv0())
         << " [options] <file> [<file> ...]"
         << endl << args.GetHelp() << endl;
    return 1;
    }

  if (outDir.length() && outDir[outDir.length() - 1] != '/')
    {
    outDir += '/';
    }

  // Process files
  vtkSmartPointer<vtkTimerLog> log = vtkSmartPointer<vtkTimerLog>::New();
  for (int i = 1; i < argc; i++)
    {
    const char* inputFile = argv[i];
    std::string outputFilePath =
      vtksys::SystemTools::GetFilenameWithoutLastExtension(inputFile) + ".mrj";
    if (outDir.size())
      {
      outputFilePath = outDir + outputFilePath;
      }
    else
      {
      std::string inputFileDir =
        vtksys::SystemTools::GetFilenamePath(inputFile);
      if (inputFileDir.size())
        {
        if (inputFileDir != "/")
          {
          inputFileDir += '/';
          }
        outputFilePath = inputFileDir + outputFilePath;
        }
      }
    log->StartTimer();

    // Last argument is the compression quality.
    ConvertToMultiResJpg(inputFile, outputFilePath.c_str(), quality);
    log->StopTimer();
    cerr << i << ", " << log->GetElapsedTime() << endl;
    }

  return 0;
}
