// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
