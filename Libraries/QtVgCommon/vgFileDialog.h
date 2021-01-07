// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgFileDialog_h
#define __vgFileDialog_h

#if defined(ENABLE_QTTESTING)

#include <pqFileDialog.h>
typedef pqFileDialog vgFileDialog;

#else

#include <QFileDialog>
typedef QFileDialog vgFileDialog;

#endif

#endif
