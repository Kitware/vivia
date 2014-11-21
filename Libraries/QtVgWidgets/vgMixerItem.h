/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgMixerItem_h
#define __vgMixerItem_h

#include <qtDrawer.h>
#include <qtGlobal.h>

class vgMixerDrawer;

class vgMixerItemPrivate;

class vgMixerItem : public qtDrawer
{
  Q_OBJECT

public:
  vgMixerItem(int key, const QString& text, vgMixerDrawer* drawer);
  virtual ~vgMixerItem();

  int key() const;
  bool state() const;
  bool isInverted() const;
  double value() const;

  double minimum() const;
  double maximum() const;
  double singleStep() const;
  double pageStep() const;

  vgMixerDrawer* parent();

  void setText(const QString&);
  void setInvertedText(const QString&);
  void setMinimum(double minimum);
  void setMaximum(double maximum);
  void setSingleStep(double step);
  void setPageStep(double step);
  void setRange(double minimum, double maximum);
  void setRange(double minimum, double maximum,
                double singleStep, double pageStep);

signals:
  void stateChanged(int key, bool);
  void invertedChanged(int key, bool);
  void valueChanged(int key, double);

public slots:
  void setState(bool);
  void setInverted(bool);
  void setValue(double);

protected slots:
  void setValueFromSlider(double);
  void setValueFromSpinBox(double);
  void emitStateChanged(bool);
  void emitInvertedChanged(bool);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgMixerItem)

private:
  QTE_DECLARE_PRIVATE(vgMixerItem)
  Q_DISABLE_COPY(vgMixerItem)
};

#endif
