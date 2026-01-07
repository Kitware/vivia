// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvFakeQueryServerChooser_h
#define __vvFakeQueryServerChooser_h

#include <qtGlobal.h>

#include "vvAbstractQueryServerChooser.h"

class QTreeWidgetItem;

class vvFakeQueryServerChooserPrivate;

class vvFakeQueryServerChooser : public vvAbstractQueryServerChooser
{
  Q_OBJECT

public:
  vvFakeQueryServerChooser(QWidget* parent = 0);
  virtual ~vvFakeQueryServerChooser();

  virtual QUrl uri() const;
  virtual void setUri(QUrl);

protected slots:
  void updateUri();
  void browseForArchive();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvFakeQueryServerChooser)

private:
  QTE_DECLARE_PRIVATE(vvFakeQueryServerChooser)
  Q_DISABLE_COPY(vvFakeQueryServerChooser)
};

#endif
