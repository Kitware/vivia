/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgTypes_h
#define __vgTypes_h

#ifdef _WIN32
#ifdef _WIN64
typedef unsigned __int64 uintptr;
#else
typedef unsigned int uintptr;
#endif
#else
typedef unsigned long uintptr;
#endif

#endif // __vgTypes_h
