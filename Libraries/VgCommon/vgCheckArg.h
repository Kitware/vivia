/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vgCheckArg_h
#define __vgCheckArg_h

#define CHECK_ARG(_expr, ...) \
  if (!(_expr)) return __VA_ARGS__

#endif
