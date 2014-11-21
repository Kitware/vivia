/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqResultFilter.h"

//-----------------------------------------------------------------------------
vqResultFilter::vqResultFilter() : Threshold(-1.0)
{
}

//-----------------------------------------------------------------------------
bool vqResultFilter::isNoop()
{
  return this->Threshold < 0.0;
}
