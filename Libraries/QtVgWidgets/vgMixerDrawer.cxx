/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgMixerDrawer.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>

#include "vgMixerWidget.h"

QTE_IMPLEMENT_D_FUNC(vgMixerDrawer)

//-----------------------------------------------------------------------------
class vgMixerDrawerPrivate
{
public:
  vgMixerDrawerPrivate();

  int id_;

  QLabel* label_;
  QCheckBox* checkbox_;
  QWidget* textWidget_;

  vgMixerDrawer* parent_;
  vgMixerWidget* widget_;
};

//-----------------------------------------------------------------------------
vgMixerDrawerPrivate::vgMixerDrawerPrivate() :
  id_(0), label_(0), checkbox_(0), textWidget_(0),  parent_(0), widget_(0)
{
}

//-----------------------------------------------------------------------------
vgMixerDrawer::vgMixerDrawer(vgMixerWidget* widget) :
  qtDrawer(widget, qobject_cast<QGridLayout*>(widget->layout())),
  d_ptr(new vgMixerDrawerPrivate)
{
  QTE_D(vgMixerDrawer);
  d->widget_ = widget;
}

//-----------------------------------------------------------------------------
vgMixerDrawer::vgMixerDrawer(int id, const QString& text,
                             vgMixerDrawer* parent, bool checkable) :
  qtDrawer(parent),
  d_ptr(new vgMixerDrawerPrivate)
{
  QTE_D(vgMixerDrawer);

  d->id_ = id;
  d->parent_ = parent;
  d->widget_ = parent->d_func()->widget_;

  if (checkable)
    {
    d->textWidget_ = d->checkbox_ = new QCheckBox(text);
    d->checkbox_->setCheckState(Qt::PartiallyChecked);
    connect(d->checkbox_, SIGNAL(clicked(bool)),
            this, SLOT(setChecked(bool)));
    }
  else
    {
    d->textWidget_ = d->label_ = new QLabel(text);
    }
  d->textWidget_->setEnabled(false);

  this->addWidget(d->textWidget_);
  this->setExpanderPolicy(qtDrawer::ExpanderAlwaysShown);
}

//-----------------------------------------------------------------------------
vgMixerDrawer::~vgMixerDrawer()
{
}

//-----------------------------------------------------------------------------
int vgMixerDrawer::id() const
{
  QTE_D_CONST(vgMixerDrawer);
  return d->id_;
}

//-----------------------------------------------------------------------------
vgMixerWidget* vgMixerDrawer::widget()
{
  QTE_D(vgMixerDrawer);
  return d->widget_;
}

//-----------------------------------------------------------------------------
vgMixerDrawer* vgMixerDrawer::parent()
{
  QTE_D(vgMixerDrawer);
  return d->parent_;
}

//-----------------------------------------------------------------------------
void vgMixerDrawer::setCheckState(Qt::CheckState state)
{
  QTE_D(vgMixerDrawer);
  if (d->checkbox_)
    {
    d->checkbox_->setCheckState(state);
    }
}

//-----------------------------------------------------------------------------
void vgMixerDrawer::setChecked(bool checked)
{
  QTE_D(vgMixerDrawer);

  if (d->checkbox_)
    {
    d->checkbox_->setChecked(checked);
    emit this->checkToggled(d->id_, checked);
    }
}

//-----------------------------------------------------------------------------
void vgMixerDrawer::addChild(qtDrawer* child, qtDrawer* before)
{
  QTE_D(vgMixerDrawer);
  if (d->textWidget_)
    {
    d->textWidget_->setEnabled(true);
    }
  qtDrawer::addChild(child, before);
}

//-----------------------------------------------------------------------------
void vgMixerDrawer::removeChild(qtDrawer* child)
{
  QTE_D(vgMixerDrawer);
  qtDrawer::removeChild(child);
  if (d->textWidget_)
    {
    if (this->countDescendants() > 0)
      {
      d->textWidget_->setEnabled(true);
      }
    else
      {
      d->textWidget_->setEnabled(false);
      if (d->checkbox_)
        {
        d->checkbox_->setCheckState(Qt::PartiallyChecked);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vgMixerDrawer::clear()
{
  QTE_D(vgMixerDrawer);
  if (d->textWidget_)
    {
    d->textWidget_->setEnabled(false);
    }
  qtDrawer::clear();
}
