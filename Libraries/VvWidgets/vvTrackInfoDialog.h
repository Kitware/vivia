/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvTrackInfoDialog_h
#define __vvTrackInfoDialog_h

#include <QDialog>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vvTrack.h>

class vvTrackInfoDialogPrivate;

class VV_WIDGETS_EXPORT vvTrackInfoDialog : public QDialog
{
  Q_OBJECT

public:
  explicit vvTrackInfoDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~vvTrackInfoDialog();

  QList<vvTrack> tracks() const;

public slots:
  void setTracks(QList<vvTrack>);
  void setTracks(std::vector<vvTrack>);
  void clearTracks();

protected slots:
  void toggleDetails(bool);
  void showDetails();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvTrackInfoDialog)

private:
  QTE_DECLARE_PRIVATE(vvTrackInfoDialog)
  Q_DISABLE_COPY(vvTrackInfoDialog)
};

#endif
