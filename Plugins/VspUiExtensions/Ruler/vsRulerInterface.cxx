/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vsRulerInterface.h"

#include <QMenu>
#include <QToolBar>

#include <QVTKWidget.h>
#include <vtkRenderWindow.h>

#include <qtActionManager.h>
#include <qtPrioritizedMenuProxy.h>
#include <qtPrioritizedToolBarProxy.h>
#include <qtScopedValueChange.h>

#include <vsCore.h>
#include <vsMainWindow.h>
#include <vsScene.h>

#include "vsLineWidget.h"

QTE_IMPLEMENT_D_FUNC(vsRulerInterface)

namespace // anonymous
{

//-----------------------------------------------------------------------------
QAction* createAction(QWidget* parent, const char* name,
                      const char* text, const char* icon = 0)
{
  const QString key("MainWindow/%1");
  QSettings settings;

  QAction* action = new QAction(parent);
  action->setObjectName(name);
  action->setText(text);
  qtAm->setupAction(settings, action, key.arg(name), icon);

  return action;
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
class vsRulerInterfacePrivate
{
public:
  explicit vsRulerInterfacePrivate(
    vsMainWindow* window, vsScene* scene, vsCore* core);

  QString buildRulerText();

  vsCore* const Core;
  vsScene* const Scene;
  vsMainWindow* const Window;

  QAction* ActionShowRuler;

  QScopedPointer<vsLineWidget> Ruler;
  double CurrentGsd;

  bool IgnoreHideRuler;
};

//-----------------------------------------------------------------------------
vsRulerInterfacePrivate::vsRulerInterfacePrivate(
  vsMainWindow* window, vsScene* scene, vsCore* core) :
  Core(core),
  Scene(scene),
  Window(window),
  IgnoreHideRuler(false)
{
}

//-----------------------------------------------------------------------------
QString vsRulerInterfacePrivate::buildRulerText()
{
  double px = this->Ruler->lineLength();

  if (this->CurrentGsd > 0.0)
    {
    const QString f = "Ruler: %1 m (%2 px)";
    return f.arg(px * this->CurrentGsd, 0, 'f', 2)
           .arg(px, 0, 'f', 2);
    }
  else
    {
    return QString("Ruler: %2 px").arg(px, 0, 'f', 2);
    }
}

//-----------------------------------------------------------------------------
vsRulerInterface::vsRulerInterface(
  vsMainWindow* window, vsScene* scene, vsCore* core) :
  QObject(window),
  d_ptr(new vsRulerInterfacePrivate(window, scene, core))
{
  QTE_D(vsRulerInterface);

  QSettings settings;

  // Create tool action
  d->ActionShowRuler = new QAction(window);
  d->ActionShowRuler->setObjectName("actionShowRuler");
  d->ActionShowRuler->setText("Show Ruler");
  d->ActionShowRuler->setIconText("Ruler");
  d->ActionShowRuler->setToolTip("Show or hide the ruler tool, used to measure"
                                 " the distance between two points");
  d->ActionShowRuler->setCheckable(true);
  qtAm->setupAction(settings, d->ActionShowRuler,
                    "MainWindow/actionShowRuler", "measure");
  connect(d->ActionShowRuler, SIGNAL(toggled(bool)),
          this, SLOT(toggleRuler(bool)));

  // Add tool action to menu and tool bar
  window->toolsMenu()->insertAction(d->ActionShowRuler, 100);
  window->toolsToolBar()->insertAction(d->ActionShowRuler, 100);

  connect(scene, SIGNAL(interactionCanceled()), this, SLOT(hideRuler()));
  connect(scene, SIGNAL(videoMetadataUpdated(vtkVgVideoFrameMetaData, qint64)),
          this, SLOT(setGsdFromMetadata(vtkVgVideoFrameMetaData)));
  connect(core, SIGNAL(videoSourceStatusChanged(vsDataSource::Status)),
          this, SLOT(setVideoSourceStatus(vsDataSource::Status)));
  connect(this, SIGNAL(statusMessageAvailable(QString)),
          window, SLOT(setStatusText(QString)));
}

//-----------------------------------------------------------------------------
vsRulerInterface::~vsRulerInterface()
{
}

//-----------------------------------------------------------------------------
void vsRulerInterface::setVideoSourceStatus(
  vsDataSource::Status status)
{
  QTE_D(vsRulerInterface);

  switch (status)
    {
    case vsDataSource::StreamingActive:
    case vsDataSource::StreamingIdle:
    case vsDataSource::StreamingStopped:
    case vsDataSource::ArchivedActive:
    case vsDataSource::ArchivedSuspended:
    case vsDataSource::ArchivedIdle:
      d->ActionShowRuler->setEnabled(true);
      break;
    default:
      d->ActionShowRuler->setChecked(false);
      d->ActionShowRuler->setEnabled(false);
      break;
    }
}

//-----------------------------------------------------------------------------
void vsRulerInterface::setGsdFromMetadata(vtkVgVideoFrameMetaData md)
{
  QTE_D(vsRulerInterface);
  d->CurrentGsd = md.Gsd;
  this->rulerLengthChanged();
}

//-----------------------------------------------------------------------------
void vsRulerInterface::toggleRuler(bool state)
{
  QTE_D(vsRulerInterface);
  if (state)
    {
    qtScopedValueChange<bool> ic(d->IgnoreHideRuler, true);
    d->Scene->cancelInteraction();

    vtkRenderWindowInteractor* const interactor =
      d->Window->view()->GetRenderWindow()->GetInteractor();
    d->Ruler.reset(new vsLineWidget(interactor));
    d->Ruler->setMatrix(d->Scene->currentTransform());
    d->Ruler->begin();

    connect(d->Scene, SIGNAL(transformChanged(QMatrix4x4)),
            d->Ruler.data(), SLOT(setMatrix(QMatrix4x4)));
    connect(d->Ruler.data(), SIGNAL(lineLengthChanged(double)),
            this, SLOT(rulerLengthChanged()));

    this->rulerLengthChanged();
    d->Scene->postUpdate();
    }
  else
    {
    d->Ruler.reset();
    d->Scene->postUpdate();
    emit this->statusMessageAvailable(QString());
    }
}

//-----------------------------------------------------------------------------
void vsRulerInterface::hideRuler()
{
  QTE_D(vsRulerInterface);
  if (!d->IgnoreHideRuler)
    {
    d->ActionShowRuler->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
void vsRulerInterface::rulerLengthChanged()
{
  QTE_D(vsRulerInterface);
  if (d->Ruler)
    {
    emit this->statusMessageAvailable(d->buildRulerText());
    }
}
