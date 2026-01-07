// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vqPredefinedQueryDialog_h
#define __vqPredefinedQueryDialog_h

#include <qtGlobal.h>
#include <qtStatusSource.h>

#include <vvQueryInstance.h>

#include <vvAbstractSimilarityQueryDialog.h>

class QUrl;

class vqPredefinedQueryDialogPrivate;

class vqPredefinedQueryDialog : public vvAbstractSimilarityQueryDialog
{
  Q_OBJECT

public:
  vqPredefinedQueryDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vqPredefinedQueryDialog();

  int similarity() const;
  virtual std::vector<vvTrack> selectedTracks() const;
  virtual std::vector<vvDescriptor> selectedDescriptors() const;
  std::vector<unsigned char> iqrModel() const;

  virtual void setSelectedDescriptors(const std::vector<vvDescriptor>&);

public slots:
  virtual int exec();

protected slots:
  void selectQuery(int id);
  void toggleClassifierGroup(bool state);
  void enableAcceptIfClassifiersSelected();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vqPredefinedQueryDialog)

private:
  QTE_DECLARE_PRIVATE(vqPredefinedQueryDialog)
  Q_DISABLE_COPY(vqPredefinedQueryDialog)
};

#endif
