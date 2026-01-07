// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
