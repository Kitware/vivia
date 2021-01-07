// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgMixerDrawer_h
#define __vgMixerDrawer_h

#include <qtDrawer.h>

class vgMixerWidget;

class vgMixerDrawerPrivate;

class vgMixerDrawer : public qtDrawer
{
  Q_OBJECT

public:
  vgMixerDrawer(vgMixerWidget* widget);
  vgMixerDrawer(int id, const QString& text, vgMixerDrawer* parent,
                bool checkable = false);
  virtual ~vgMixerDrawer();

  int id() const;
  vgMixerWidget* widget();
  vgMixerDrawer* parent();

signals:
  void checkToggled(int id, bool);

public slots:
  void setCheckState(Qt::CheckState);

protected slots:
  virtual void setChecked(bool);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgMixerDrawer)

  virtual void addChild(qtDrawer* child, qtDrawer* before = 0);
  virtual void removeChild(qtDrawer* child);
  virtual void clear();

private:
  QTE_DECLARE_PRIVATE(vgMixerDrawer)
  Q_DISABLE_COPY(vgMixerDrawer)
};

#endif
