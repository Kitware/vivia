/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
