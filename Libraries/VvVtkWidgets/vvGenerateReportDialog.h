/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
