/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vtkstd/string>
#include <vtksys/CommandLineArguments.hxx>
#include <vtksys/Directory.hxx>
#include <vtksys/Glob.hxx>
#include <vtksys/Process.h>
#include <vtksys/SystemTools.hxx>

#include <vtkImageData.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>

#include <vtkVgImageSource.h>
#include <vtkVgMultiResJpgImageReader2.h>

#ifdef _WIN32
  #define strcasecmp _stricmp
#endif

#include <cstdlib>

//----------------------------------------------------------------------------
void ConvertToPNG(const char* inFile, const char* outFile, int level)
{
  vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName(outFile);

  vtkStdString ext = vtksys::SystemTools::GetFilenameLastExtension(inFile);

  if (strcasecmp(ext.c_str(), ".mrj") == 0)
    {
    vtkSmartPointer<vtkVgMultiResJpgImageReader2> reader =
      vtkSmartPointer<vtkVgMultiResJpgImageReader2>::New();
    reader->SetFileName(inFile);
    reader->SetLevel(level);
    writer->SetInput(reader->GetOutput());
    }
  else
    {
    std::cerr << "Files of type " << ext << " are not handled as of now."
              << std::endl;
    exit(1);
    }

  writer->Write();
}

//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  vtksys::CommandLineArguments args;
  args.Initialize(argc, argv);
  args.StoreUnusedArguments(1);

  int level = 0;
  vtkStdString outDir;

  args.AddArgument("-l", vtksys::CommandLineArguments::SPACE_ARGUMENT,
                   &level, "Level of Detail (0-5; default 0 (highest))");
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
  for (int i = 1; i < argc; i++)
    {
    const char* inputFile = argv[i];
    vtkStdString outputFilePath =
      vtksys::SystemTools::GetFilenameWithoutLastExtension(inputFile) + ".png";
    if (outDir.size())
      {
      outputFilePath = outDir + outputFilePath;
      }
    else
      {
      vtkStdString inputFileDir =
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

    // Last argument is the compression quality.
    ConvertToPNG(inputFile, outputFilePath.c_str(), level);
    }

  return 0;
}
