// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsFakeStreamFactory_h
#define __vsFakeStreamFactory_h

#include <vsStreamFactory.h>

class vsFakeStreamFactory : public vsStreamFactory
{
public:
  vsFakeStreamFactory();
  virtual ~vsFakeStreamFactory();

  virtual bool initialize(QWidget* dialogParent) QTE_OVERRIDE;
  virtual bool initialize(const QUrl& uri) QTE_OVERRIDE;

protected:
  bool initialize(const QUrl& uri, QWidget* dialogParent);

private:
  QTE_DISABLE_COPY(vsFakeStreamFactory)
};

#endif
