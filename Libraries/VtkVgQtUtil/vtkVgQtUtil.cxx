// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgQtUtil.h"

#include <QApplication>
#include <QThreadStorage>

#include <vtkEventQtSlotConnect.h>

#include <vtkVgInstance.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
class vtkVgQtConnectionManager : public QObject
{
public:
  vtkVgQtConnectionManager() {}
  virtual ~vtkVgQtConnectionManager() {}

  vtkEventQtSlotConnect* manager();

protected:
  typedef vtkVgInstance<vtkEventQtSlotConnect> Manager;
  QThreadStorage<Manager*> tls;
};

Q_GLOBAL_STATIC(vtkVgQtConnectionManager, globalConnectionManager)

//-----------------------------------------------------------------------------
vtkEventQtSlotConnect* vtkVgQtConnectionManager::manager()
{
  if (!this->tls.hasLocalData())
    {
    this->tls.setLocalData(new Manager);
    }
  return *this->tls.localData();
}

} // namespace <anonymous>

//-----------------------------------------------------------------------------
void vtkConnect(
  vtkObject* sender, unsigned long event, QObject* receiver, const char* slot,
  Qt::ConnectionType type)
{
  vtkConnect(sender, event, receiver, slot, 0, 0.0f, type);
}

//-----------------------------------------------------------------------------
void vtkConnect(
  vtkObject* sender, unsigned long event, QObject* receiver, const char* slot,
  void* data, float priority, Qt::ConnectionType type)
{
  vtkEventQtSlotConnect* m = globalConnectionManager()->manager();
  m->Connect(sender, static_cast<vtkCommand::EventIds>(event),
             receiver, slot, data, priority, type);
}

//-----------------------------------------------------------------------------
void vtkDisconnect(
  vtkObject* sender, unsigned long event,
  QObject* receiver, const char* slot,
  void* data)
{
  vtkEventQtSlotConnect* m = globalConnectionManager()->manager();
  m->Disconnect(sender, event, receiver, slot, data);
}
