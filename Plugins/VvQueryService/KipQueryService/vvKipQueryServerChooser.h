/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvKipQueryServerChooser_h
#define __vvKipQueryServerChooser_h

#include <qtGlobal.h>

#include "vvAbstractQueryServerChooser.h"

class QTreeWidgetItem;

class vvKipQueryServerChooserPrivate;

class vvKipQueryServerChooser : public vvAbstractQueryServerChooser
{
  Q_OBJECT

public:
  vvKipQueryServerChooser(QWidget* parent = 0);
  virtual ~vvKipQueryServerChooser();

  virtual QUrl uri() const;
  virtual void setUri(QUrl);

protected slots:
  void updateUri();
  void browseForPipeline();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vvKipQueryServerChooser)

private:
  QTE_DECLARE_PRIVATE(vvKipQueryServerChooser)
  Q_DISABLE_COPY(vvKipQueryServerChooser)
};

#endif
