// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgMixerWidget_h
#define __vgMixerWidget_h

#include <QList>

#include <qtDrawerWidget.h>
#include <qtGlobal.h>

#include <vgExport.h>

class vgMixerWidgetPrivate;

class QTVG_WIDGETS_EXPORT vgMixerWidget : public qtDrawerWidget
{
  Q_OBJECT

public:
  vgMixerWidget(QWidget* parent = 0);
  virtual ~vgMixerWidget();

  virtual int beginGroup(const QString& text, bool checkable = false);
  virtual void endGroup();

  virtual int addGroup(const QString& text, int parent = 0);
  virtual int addGroup(const QString& text, bool checkable, int parent = 0);
  virtual bool addItem(int key, const QString& text, int group = -1);
  virtual bool removeGroup(int group);
  virtual bool removeItem(int key);
  virtual void clear();

  virtual QList<int> keys();

  virtual bool state(int key) const;
  virtual bool isInverted(int key) const;
  virtual double value(int key) const;
  virtual double minimum(int key) const;
  virtual double maximum(int key) const;
  virtual double singleStep(int key) const;
  virtual double pageStep(int key) const;

  virtual void setText(int key, const QString& text);
  virtual void setInvertedText(int key, const QString& text);
  virtual void setMinimum(int key, double minimum);
  virtual void setMaximum(int key, double maximum);
  virtual void setSingleStep(int key, double step);
  virtual void setPageStep(int key, double step);
  virtual void setRange(int key, double minimum, double maximum);
  virtual void setRange(int key, double minimum, double maximum,
                        double singleStep, double pageStep);

signals:
  void stateChanged(int key, bool state);
  void invertedChanged(int key, bool state);
  void valueChanged(int key, double value);

public slots:
  virtual void setState(int key, bool state);
  virtual void setInverted(int key, bool state);
  virtual void setValue(int key, double value);

  virtual void setGroupVisible(int group, bool visible);
  virtual void setGroupState(int group, bool state);
  virtual void setGroupInverted(int group, bool state);
  virtual void setGroupValue(int group, double value);

  virtual void setExpanded(int group, bool expanded = true);

protected slots:
  virtual void updateItemParentState(int key);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vgMixerWidget)

  virtual qtDrawer* createRoot();

private:
  QTE_DECLARE_PRIVATE(vgMixerWidget)
  Q_DISABLE_COPY(vgMixerWidget)
};

#endif
