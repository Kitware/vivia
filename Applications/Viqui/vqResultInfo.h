// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
