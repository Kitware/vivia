// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
