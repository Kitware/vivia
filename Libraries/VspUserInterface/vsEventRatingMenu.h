/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
