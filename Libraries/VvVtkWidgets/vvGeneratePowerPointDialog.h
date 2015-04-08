/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
