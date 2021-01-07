// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
