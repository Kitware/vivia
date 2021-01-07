// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vdfAbstractTrackNode_h
#define __vdfAbstractTrackNode_h

#include <QObject>

class vtkVgTrackModel;

/// Interface for a data node providing an track model.
class vdfAbstractTrackNode
{
public:
  virtual ~vdfAbstractTrackNode() {}

  /// Get track model.
  ///
  /// \return Pointer to the node's provided track model.
  ///
  /// The returned pointer shall remain valid as long as the node is in use,
  /// but may become invalid once the node becomes unused.
  virtual vtkVgTrackModel* trackModel() const = 0;

protected:
  vdfAbstractTrackNode() {}
};

Q_DECLARE_INTERFACE(vdfAbstractTrackNode,
                    "org.visgui.framework.data.trackNode")

#endif
