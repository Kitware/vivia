// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
