/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgFrameScrubber_h
#define __vgFrameScrubber_h

#include <QAbstractSlider>

#include <qtGlobal.h>

#include <vgExport.h>

class vgFrameScrubberPrivate;

class QTVG_WIDGETS_EXPORT vgFrameScrubber : public QWidget
{
  Q_OBJECT

public:
  explicit vgFrameScrubber(QWidget* parent = 0);
  virtual ~vgFrameScrubber();

signals:
  void timeChanged(double value);
  void timesChanged(double minVal, double maxVal, bool resized);
  void frameNumberChanged(int value);
  void frameNumbersChanged(int minVal, int maxVal, bool resized);

  void minTimeChanged(double value);
  void minFrameNumberChanged(int value);

  void frameNumberRangeChanged(int min, int max);
  void timeRangeChanged(double min, double max);

public slots:
  void setFrameNumberRange(int minimumFrameNumber, int maximumFrameNumber);
  void setFrameNumber(int value);
  void setFrameNumbers(int minVal, int maxVal);
  void setMinFrameNumber(int value);

  void setTimeRange(double minimumTime, double maximumTime,
                    double resolution = 1e3);
  void setTime(double value);
  void setTimes(double minVal, double maxVal);
  void setMinTime(double value);

  void setIntervalModeEnabled(bool enable);

protected slots:
  void updateValueFromSlider(int);
  void updateValuesFromSlider(int, int);

protected:
  QTE_DECLARE_PRIVATE_PTR(vgFrameScrubber)

private:
  QTE_DECLARE_PRIVATE(vgFrameScrubber)
  Q_DISABLE_COPY(vgFrameScrubber)
};

#endif
