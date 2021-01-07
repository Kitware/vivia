// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvGenerateReportDialog_h
#define __vvGenerateReportDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include <vgExport.h>

class vvGenerateReportDialogPrivate;

class VV_VTKWIDGETS_EXPORT vvGenerateReportDialog : public QDialog
{
  Q_OBJECT

public:
  explicit vvGenerateReportDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~vvGenerateReportDialog();

  bool generateVideo();
  QString outputPath();

public slots:
  virtual void accept();

protected slots:
  void browseForOutputPath();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvGenerateReportDialog)

private:
  QTE_DECLARE_PRIVATE(vvGenerateReportDialog)
};

#endif // __vvGenerateReportDialog_h
