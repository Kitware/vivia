/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqVideoQueryDialog_h
#define __vqVideoQueryDialog_h

#include <qtGlobal.h>

#include <vvDescriptor.h>
#include <vvQueryFormulation.h>

#include <vvVideoQueryDialog.h>

class vqVideoQueryDialogPrivate;

class vqVideoQueryDialog : public vvVideoQueryDialog
{
  Q_OBJECT

public:
  vqVideoQueryDialog(bool useAdvancedUi, QWidget* parent = 0,
                     Qt::WindowFlags flags = 0);
  ~vqVideoQueryDialog();

  virtual void setSelectedDescriptors(const std::vector<vvDescriptor>&);

  std::string exemplarUri() const;

  void setExemplarUri(const std::string& uri, vvQueryFormulationType type);

  void setTimeRange(double startTime, double endTime, double initialTime);

signals:
  void queryFormulationRequested(vvProcessingRequest,
                                 bool bypassCache,
                                 qtStatusManager* statusTarget);

public slots:
  virtual int exec();

protected slots:
  virtual void chooseQueryVideo();
  virtual void reprocessQueryVideo();

  virtual void setQueryTracksAndDescriptors(
    QList<vvDescriptor> descriptors, QList<vvTrack> tracks);

  virtual void clearQueryDescriptors();

protected:
  void processQueryVideo(bool bypassCache);

  QList<vvDescriptor> filterDescriptors(
    QList<vvDescriptor> descriptors);

private:
  QTE_DECLARE_PRIVATE(vqVideoQueryDialog)
};

#endif
