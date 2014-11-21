/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpTrackColorDialog_h
#define __vpTrackColorDialog_h

#include <QDialog>

#include <qtGlobal.h>

class vgAttributeSet;

class vpTrackColorDialogPrivate;

class vpTrackColorDialog : public QDialog
{
  Q_OBJECT

public:
  enum ColoringMode
    {
    SingleColor,
    RandomColor,
    ColorByTypeLabel,
    ColorByStatePVO,
    ColorByStateAttribute
    };

  vpTrackColorDialog(const vgAttributeSet* trackAttribs = 0,
                     QWidget* parent = 0);
  ~vpTrackColorDialog();

  ColoringMode mode() const;
  void setMode(ColoringMode);

  QString attributeGroup() const;
  void setAttributeGroup(const QString& attributeGroup);

signals:
  void updateRequested(vpTrackColorDialog* dlg);

public slots:
  virtual void accept();

protected slots:
  void setAttrColors();
  void apply();
  void applyAttributeColors();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpTrackColorDialog)

private:
  QTE_DECLARE_PRIVATE(vpTrackColorDialog)
  Q_DISABLE_COPY(vpTrackColorDialog)
};

#endif
