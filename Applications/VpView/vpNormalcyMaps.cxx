// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpNormalcyMaps.h"

#ifdef VISGUI_USE_VIDTK
#include <utilities/token_expansion.h>
#endif

// Qt includes.
#include <QDebug>
#include <QFileInfo>
#include <QMap>
#include <QSettings>
#include <QStringList>

// VTK includes.
#include <vtkImageData.h>
#include <vtkPNGReader.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

// QtExtensions includes.
#include <qtStlUtil.h>

// C++ includes.
#include <vector>

//-----------------------------------------------------------------------------
class vpNormalcyMaps::vpNormalcyMapsInternal
{
public:
  vpNormalcyMapsInternal() :
    PrecomputeNormalcy(0)
    {
    }

  ~vpNormalcyMapsInternal()
    {
    }

  void ReadNormalcyMaps(QSettings& settings);

  vtkSmartPointer<vtkImageData> CreateImageData(const QString& source);

  int PrecomputeNormalcy;

  typedef QMap<QString, QString>                        NormalcyMapType;
  typedef QMap<QString, vtkSmartPointer<vtkImageData> > NormalcyImageMapType;
  typedef QMap<QString, vtkSmartPointer<vtkActor> >     NormalcyActorMapType;

  typedef NormalcyMapType::iterator             NormalcyMapItr;
  typedef NormalcyMapType::const_iterator       NormalcyMapConstItr;
  typedef NormalcyImageMapType::iterator        NormalcyImageMapItr;
  typedef NormalcyImageMapType::const_iterator  NormalcyImageMapConstItr;

  NormalcyMapType       NormalcyMap;
  NormalcyImageMapType  NormalcyImageMap;
  NormalcyActorMapType  NormalcyActorMap;
};

//-----------------------------------------------------------------------------
void vpNormalcyMaps::vpNormalcyMapsInternal::ReadNormalcyMaps(QSettings& settings)
{
  settings.beginGroup("NormalcyMaps");
  int size = settings.beginReadArray("NormalcyMaps");

  for (int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);
    QString type = settings.value("Key").toString();
    QString source = settings.value("Source").toString();

    // Expand tokens if any.
#ifdef VISGUI_USE_VIDTK
    source = qtString(vidtk::token_expansion::expand_token(stdString(source)));
#endif

    if (!this->PrecomputeNormalcy)
      {
      this->NormalcyMap.insert(type, source);
      }
    else
      {
      // \todo: Implement this.
      }
    }
  settings.endArray();
  settings.endGroup();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData>
vpNormalcyMaps::vpNormalcyMapsInternal::CreateImageData(const QString& source)
{
  if (!QFileInfo(source).exists())
    {
    qDebug() << "Normalcy file not found:" << source;
    return 0;
    }

  const char* filename = stdString(source).c_str();
  QString ext = qtString(vtksys::SystemTools::GetFilenameLastExtension(filename));
  if (QString::compare("png", ext))
    {
    qDebug() << "Format " << ext << "is not supported";
    return 0;
    }

  vtkSmartPointer<vtkPNGReader> reader(vtkSmartPointer<vtkPNGReader>::New());
  reader->SetFileName(filename);
  reader->Update();

  vtkSmartPointer<vtkImageData> output(vtkSmartPointer<vtkImageData>::New());
  output->ShallowCopy(reader->GetOutput());
  return output;
}

//-----------------------------------------------------------------------------
vpNormalcyMaps::vpNormalcyMaps()
{
  this->Internal = new vpNormalcyMapsInternal();
}

//-----------------------------------------------------------------------------
vpNormalcyMaps::~vpNormalcyMaps()
{
  delete this->Internal;

  this->Internal = 0;
}

//-----------------------------------------------------------------------------
int vpNormalcyMaps::LoadFromFile(const QString& filename)
{
  if (!QFileInfo(filename).exists())
    {
    qDebug() << "Normalcy maps file not found:" << filename;
    return 0;
    }

  QSettings settings(filename, QSettings::IniFormat);
  this->Internal->ReadNormalcyMaps(settings);
  return 1;
}

//-----------------------------------------------------------------------------
void vpNormalcyMaps::SetPrecompute(int state)
{
  this->Internal->PrecomputeNormalcy = state;
}

//-----------------------------------------------------------------------------
int vpNormalcyMaps::GetPrecompute() const
{
  return this->Internal->PrecomputeNormalcy;
}

//-----------------------------------------------------------------------------
QString vpNormalcyMaps::GetNormalcySource(const QString& key) const
{
  vpNormalcyMapsInternal::NormalcyMapConstItr constItr =
    this->Internal->NormalcyMap.find(key);
  if (constItr != this->Internal->NormalcyMap.end())
    {
    return constItr.value();
    }
  else
    {
    return QString("");
    }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData>
vpNormalcyMaps::GetNormalcyImage(const QString& key) const
{
  vpNormalcyMapsInternal::NormalcyImageMapConstItr imageMapItr =
    this->Internal->NormalcyImageMap.find(key);

  if (imageMapItr != this->Internal->NormalcyImageMap.end())
    {
    return imageMapItr.value();
    }
  else
    {
    vpNormalcyMapsInternal::NormalcyMapConstItr constItr =
      this->Internal->NormalcyMap.find(key);

    return this->Internal->CreateImageData(constItr.key());
    }
}

//-----------------------------------------------------------------------------
void vpNormalcyMaps::GetAllNormalcyKeys(QList<QString>& keys) const
{
  keys = this->Internal->NormalcyMap.uniqueKeys();
}

//-----------------------------------------------------------------------------
QList<vtkSmartPointer<vtkActor> > vpNormalcyMaps::GetAllNormalcyActors() const
{
  // \todo: Implement this.
  QList< vtkSmartPointer<vtkActor> > actorList;

  return actorList;
}
