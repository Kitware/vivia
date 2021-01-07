// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vvKmlWriter_h
#define __vvKmlWriter_h

#include <QList>

#include <vgExport.h>

class QFile;

class vvKmlLine;

//-----------------------------------------------------------------------------
class VV_IO_EXPORT vvKmlWriter
{
public:
  vvKmlWriter();
  ~vvKmlWriter();

  bool isEmpty() const;

  vvKmlLine* createLine();

  int write(QFile& file);

protected:
  QList<vvKmlLine*> Lines;
};

#endif
