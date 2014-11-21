/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpMergeTracksDialog_h
#define __vpMergeTracksDialog_h

#include <QDialog>

#include <qtGlobal.h>

class vpMergeTracksDialogPrivate;
class vpViewCore;
class vtkVgEvent;
class vtkVgTimeStamp;

class vpMergeTracksDialog : public QDialog
{
  Q_OBJECT

public:
  vpMergeTracksDialog(vpViewCore* coreInstance, int session,
                      QWidget* parent = 0, Qt::WindowFlags flags = 0);
  ~vpMergeTracksDialog();

  void initialize();

signals:
  void tracksMerged(int mergedId);

public slots:
  void addTrack(int trackId, int session);

  virtual void accept();

protected slots:
  void changeMergeType(bool replaceMerged);
  void moveTrackUp();
  void moveTrackDown();
  void removeTrack();
  void showAdvancedOptions();
  void validate();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vpMergeTracksDialog)

  void updateTrackTimeRanges();
  void updateOptionSettings();

  int  itemTrackId(int index);

  bool fixTrackReferences(vtkVgEvent* event, int trackId, int newTrackId);

  bool extendEventTrack(vtkVgEvent* event, int trackId,
                        const vtkVgTimeStamp& prevTrackStart,
                        const vtkVgTimeStamp& prevTrackEnd,
                        const vtkVgTimeStamp& extendedStart,
                        const vtkVgTimeStamp& extendedEnd);

  bool mergeEvents(std::vector<vtkVgEvent*>& events, bool checkOnly = false);

private:
  QTE_DECLARE_PRIVATE(vpMergeTracksDialog)
};

#endif // __vpMergeTracksDialog_h
