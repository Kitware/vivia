// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvAbstractQueryServerChooser_h
#define __vvAbstractQueryServerChooser_h

#include <QUrl>
#include <QWidget>

#include <vgExport.h>

class VV_WIDGETS_EXPORT vvAbstractQueryServerChooser : public QWidget
{
  Q_OBJECT

public:
  vvAbstractQueryServerChooser(QWidget* parent = 0) : QWidget(parent) {}
  virtual ~vvAbstractQueryServerChooser() {}

  virtual QUrl uri() const = 0;

signals:
  void uriChanged(QUrl);

public slots:
  virtual void setUri(QUrl) = 0;

private:
  Q_DISABLE_COPY(vvAbstractQueryServerChooser)
};

#endif
