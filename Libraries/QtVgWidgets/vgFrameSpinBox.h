// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgFrameSpinBox_h
#define __vgFrameSpinBox_h

#include <QSpinBox>

#include <qtGlobal.h>

#include <vgExport.h>

#include <vgNamespace.h>

class vgFrameSpinBoxPrivate;

class QTVG_WIDGETS_EXPORT vgFrameSpinBox : public QSpinBox
{
  Q_OBJECT

public:
  explicit vgFrameSpinBox(QWidget* parent = 0);
  virtual ~vgFrameSpinBox();

  void setRange(int minimum, int maximum);

signals:
  void valueChanged(int, vg::SeekMode);

protected slots:
  void emitValueChangeWithDirection(int);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgFrameSpinBox)

  virtual void stepBy(int steps);
  virtual int valueFromText(const QString& text) const;
  virtual void fixup(QString& str) const;

  using QSpinBox::valueChanged;

private:
  QTE_DECLARE_PRIVATE(vgFrameSpinBox)
  Q_DISABLE_COPY(vgFrameSpinBox)
};

#endif
