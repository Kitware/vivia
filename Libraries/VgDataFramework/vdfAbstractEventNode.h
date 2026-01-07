// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfAbstractEventNode_h
#define __vdfAbstractEventNode_h

#include <QObject>

class vtkVgEventModel;

/// Interface for a data node providing an event model.
class vdfAbstractEventNode
{
public:
  virtual ~vdfAbstractEventNode() {}

  /// Get event model.
  ///
  /// \return Pointer to the node's provided event model.
  ///
  /// The returned pointer shall remain valid as long as the node is in use,
  /// but may become invalid once the node becomes unused.
  virtual vtkVgEventModel* eventModel() const = 0;

protected:
  vdfAbstractEventNode() {}
};

Q_DECLARE_INTERFACE(vdfAbstractEventNode,
                    "org.visgui.framework.data.eventNode")

#endif
