// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
