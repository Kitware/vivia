// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <QApplication>

#include "../vvScoreGradientWidget.h"

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  vvScoreGradientWidget widget;

  widget.show();
  return app.exec();
}
