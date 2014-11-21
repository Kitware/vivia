/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsEventRatingMenu.h"

#include <QMenu>

#include <qtUtil.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
const char* getObjectName(vsEventRatingMenu::RatingMenuItem item)
{
  switch (item)
    {
    case vsEventRatingMenu::Unrated:  return "unrated";
    case vsEventRatingMenu::Relevant: return "relevant";
    case vsEventRatingMenu::Excluded: return "excluded";
    case vsEventRatingMenu::Rejected: return "rejected";
    }
  return 0;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
void vsEventRatingMenu::buildMenu(QMenu* menu)
{
  QAction* unrated = menu->addAction("Unrated / Verified");
  unrated->setObjectName(getObjectName(Unrated));
  unrated->setIcon(qtUtil::standardActionIcon("apply"));

  QAction* relevant = menu->addAction("Relevant / Verified");
  relevant->setObjectName(getObjectName(Relevant));
  relevant->setIcon(qtUtil::standardActionIcon("okay"));

  QAction* excluded = menu->addAction("Not Relevant / Verified");
  excluded->setObjectName(getObjectName(Excluded));
  excluded->setIcon(qtUtil::standardActionIcon("cancel"));

  menu->addSeparator();

  QAction* rejected = menu->addAction("Not Relevant / Rejected");
  rejected->setObjectName(getObjectName(Rejected));
  rejected->setIcon(qtUtil::standardActionIcon("delete"));
}

//-----------------------------------------------------------------------------
QAction* vsEventRatingMenu::getAction(QMenu* menu, RatingMenuItem item)
{
  QString name(getObjectName(item));
  foreach (QAction* action, menu->actions())
    {
    if (action->objectName() == name)
      {
      return action;
      }
    }
  return 0;
}
