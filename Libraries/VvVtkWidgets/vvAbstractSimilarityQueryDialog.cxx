/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvAbstractSimilarityQueryDialog.h"

#include <vvTrack.h>

//-----------------------------------------------------------------------------
vvAbstractSimilarityQueryDialog::vvAbstractSimilarityQueryDialog(
  QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
}

//-----------------------------------------------------------------------------
vvAbstractSimilarityQueryDialog::~vvAbstractSimilarityQueryDialog()
{
}

//-----------------------------------------------------------------------------
std::vector<vvTrack> vvAbstractSimilarityQueryDialog::selectedTracks() const
{
  return std::vector<vvTrack>();
}

//-----------------------------------------------------------------------------
int vvAbstractSimilarityQueryDialog::exec()
{
  return QDialog::exec();
}
