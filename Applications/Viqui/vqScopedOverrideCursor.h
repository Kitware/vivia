/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vqScopedOverrideCursor_h
#define __vqScopedOverrideCursor_h

#include <QApplication>
#include <QCursor>

struct vqScopedOverrideCursor
{
  vqScopedOverrideCursor(const QCursor& cursor = QCursor(Qt::WaitCursor))
    {
    QApplication::setOverrideCursor(cursor);
    }

  ~vqScopedOverrideCursor()
    {
    QApplication::restoreOverrideCursor();
    }
};

#endif // __vqScopedOverrideCursor_h
