/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvAbstractSimilarityQueryDialog_h
#define __vvAbstractSimilarityQueryDialog_h

#include <QDialog>

#include <vgExport.h>

struct vvDescriptor;
struct vvTrack;

class VV_VTKWIDGETS_EXPORT vvAbstractSimilarityQueryDialog : public QDialog
{
  Q_OBJECT

public:
  vvAbstractSimilarityQueryDialog(QWidget* parent = 0,
                                  Qt::WindowFlags flags = 0);
  virtual ~vvAbstractSimilarityQueryDialog();

  virtual std::vector<vvTrack> selectedTracks() const;
  virtual std::vector<vvDescriptor> selectedDescriptors() const = 0;

  virtual void setSelectedDescriptors(const std::vector<vvDescriptor>&) = 0;

public slots:
  virtual int exec();

private:
  Q_DISABLE_COPY(vvAbstractSimilarityQueryDialog)
};

#endif
