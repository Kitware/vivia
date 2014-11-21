/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqResultInfo_h
#define __vqResultInfo_h

#include <QWidget>

namespace Ui
{
class resultInfo;
}

struct vvQueryResult;

class vqResultInfo : public QWidget
{
  Q_OBJECT

public:
  vqResultInfo(QWidget* parent = 0);
  ~vqResultInfo();

public slots:
  void clear();
  void setResults(QList<const vvQueryResult*> results);

protected slots:
  void setDetailsVisible(bool);

private:
  Ui::resultInfo* UI;
};

#endif // __vqResultInfo_h
