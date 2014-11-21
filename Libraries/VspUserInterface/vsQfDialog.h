/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
