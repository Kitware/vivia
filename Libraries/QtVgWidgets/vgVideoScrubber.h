// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgVideoScrubber_h
#define __vgVideoScrubber_h

#include <QWidget>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vgNamespace.h>
#include <vgTimeStamp.h>

class vgVideoScrubberPrivate;

class QTVG_WIDGETS_EXPORT vgVideoScrubber : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(vgTimeStamp minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(vgTimeStamp maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(vgTimeStamp singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(vgTimeStamp wheelStep READ wheelStep WRITE setWheelStep)
  Q_PROPERTY(vgTimeStamp pageStep READ pageStep WRITE setPageStep)
  Q_PROPERTY(vgTimeStamp value READ value WRITE setValue NOTIFY valueChanged
                               USER true)

public:
  explicit vgVideoScrubber(QWidget* parent = 0);
  virtual ~vgVideoScrubber();

  vgTimeStamp minimum() const;
  vgTimeStamp maximum() const;

  vgTimeStamp singleStep() const;
  vgTimeStamp wheelStep() const;
  vgTimeStamp pageStep() const;

  vgTimeStamp value() const;

  bool isRangeEmpty() const;

signals:
  void valueChanged(vgTimeStamp, vg::SeekMode);
  void rangeChanged(vgTimeStamp min, vgTimeStamp max);

public slots:
  void setMinimum(vgTimeStamp);
  void setMaximum(vgTimeStamp);
  void setRange(vgTimeStamp min, vgTimeStamp max);

  void setSingleStep(vgTimeStamp);
  void setWheelStep(vgTimeStamp);
  void setPageStep(vgTimeStamp);

  void setValue(vgTimeStamp, vg::SeekMode = vg::SeekExact);

protected slots:
  void setDirectionFromSliderAction(int action);
  void updateValue();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgVideoScrubber)

  virtual void resizeEvent(QResizeEvent*);
  virtual void wheelEvent(QWheelEvent*);

private:
  QTE_DECLARE_PRIVATE(vgVideoScrubber)
  Q_DISABLE_COPY(vgVideoScrubber)
};

#endif
