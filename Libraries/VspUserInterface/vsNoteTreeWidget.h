/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsNoteTreeWidget_h
#define __vsNoteTreeWidget_h

#include <qtGlobal.h>

#include <QModelIndex>
#include <QWidget>

class QAbstractItemModel;

class vsScene;

class vsNoteTreeWidgetPrivate;

class vsNoteTreeWidget : public QWidget
{
  Q_OBJECT

public:
  vsNoteTreeWidget(QWidget* parent);
  ~vsNoteTreeWidget();

  /// Connect scene to note tree.
  ///
  /// This connects the note tree to the specified vsScene. This is used to
  /// coordinate the display of active vs. inactive notes with the display time
  /// of the specified scene.
  void connectToScene(vsScene*);

  /// Set the logical data model for the contained tree view.
  void setModel(QAbstractItemModel*);

protected slots:
  void updateSelection();
  void activate(const QModelIndex&);

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsNoteTreeWidget)

  virtual void contextMenuEvent(QContextMenuEvent*) QTE_OVERRIDE;

private:
  QTE_DECLARE_PRIVATE(vsNoteTreeWidget)
  Q_DISABLE_COPY(vsNoteTreeWidget)
};

#endif
