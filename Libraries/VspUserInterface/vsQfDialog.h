// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsQfDialog_h
#define __vsQfDialog_h

#include <qtGlobal.h>

#include <vvDescriptor.h>
#include <vvQueryFormulation.h>

#include <vvVideoQueryDialog.h>

class vsQfVideoSource;

class vsQfDialogPrivate;

struct vtkVgVideoMetadata;

class vsQfDialog : public vvVideoQueryDialog
{
  Q_OBJECT

public:
  vsQfDialog(vsQfVideoSource* source,
             QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vsQfDialog();

public slots:
  virtual int exec();

public slots:
  virtual void setQueryTracksAndDescriptors(
    QList<vvDescriptor> descriptors, QList<vvTrack> tracks);
  virtual void clearQueryDescriptors();

private:
  QTE_DECLARE_PRIVATE(vsQfDialog)
};

#endif
