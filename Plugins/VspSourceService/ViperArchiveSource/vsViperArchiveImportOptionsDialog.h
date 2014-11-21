/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsViperArchiveImportOptionsDialog_h
#define __vsViperArchiveImportOptionsDialog_h

#include <QDialog>

#include <qtGlobal.h>

class QUrl;

class vsViperArchiveImportOptionsDialogPrivate;

class vsViperArchiveImportOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  vsViperArchiveImportOptionsDialog(QWidget* parent = 0,
                                    Qt::WindowFlags flags = 0);
  ~vsViperArchiveImportOptionsDialog();

  QUrl metaDataSource() const;
  int frameOffset() const;
  double frameRate() const;
  bool importEvents() const;

  virtual void accept() QTE_OVERRIDE;

protected slots:
  void browseForVideo();
  void checkVideo();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsViperArchiveImportOptionsDialog)

private:
  QTE_DECLARE_PRIVATE(vsViperArchiveImportOptionsDialog)
  QTE_DISABLE_COPY(vsViperArchiveImportOptionsDialog)
};

#endif
