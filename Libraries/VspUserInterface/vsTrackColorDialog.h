/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsTrackColorDialog_h
#define __vsTrackColorDialog_h

#include <QDialog>

#include <qtGlobal.h>

class vsTrackColorDialogPrivate;

class vsTrackColorDialog : public QDialog
{
  Q_OBJECT

public:
  enum ColoringMode
    {
    SingleColor,
    ColorByClassification,
    ColorByDynamicData
    };

  vsTrackColorDialog(QWidget* parent = 0);
  ~vsTrackColorDialog();

  ColoringMode mode() const;
  void setMode(ColoringMode);

  QString dynamicDataSet();
  void setDynamicDataSet(const QString&);
  void setDynamicDataSets(const QStringList&);

public slots:
  virtual void accept();

  void addSource();
  void removeSources();

  void updateSourceButtonsState();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsTrackColorDialog)

private:
  QTE_DECLARE_PRIVATE(vsTrackColorDialog)
  Q_DISABLE_COPY(vsTrackColorDialog)
};

#endif
