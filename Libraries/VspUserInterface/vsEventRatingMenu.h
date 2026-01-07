// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

class QAction;
class QMenu;

namespace vsEventRatingMenu
{
  enum RatingMenuItem
    {
    Unrated,
    Relevant,
    Excluded,
    Rejected
    };

  void buildMenu(QMenu* menu);

  QAction* getAction(QMenu* menu, RatingMenuItem item);
}
