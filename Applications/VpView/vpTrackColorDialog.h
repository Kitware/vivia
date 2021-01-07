// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
    ColorByTypeClassifier,
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
