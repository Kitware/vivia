/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
