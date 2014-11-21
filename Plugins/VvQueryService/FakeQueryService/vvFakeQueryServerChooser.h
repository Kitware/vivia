/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
