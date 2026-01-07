// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvGeneratePowerPointDialog_h
#define __vvGeneratePowerPointDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include <vgExport.h>

class vvGeneratePowerPointDialogPrivate;

class VV_VTKWIDGETS_EXPORT vvGeneratePowerPointDialog : public QDialog
{
  Q_OBJECT

public:
  explicit vvGeneratePowerPointDialog(
    QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~vvGeneratePowerPointDialog();

  bool getLastConfig();

  bool generateVideo() const;
  QString outputPath() const;
  QString templateFile() const;

public slots:
  virtual void accept() QTE_OVERRIDE;

protected slots:
  void browseForOutputPath();
  void browseForTemplateFile();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvGeneratePowerPointDialog)

private:
  QTE_DECLARE_PRIVATE(vvGeneratePowerPointDialog)
};

#endif // __vvGeneratePowerPointDialog_h
