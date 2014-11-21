#ifndef __vpGraphSortAlgorithms_h
#define __vpGraphSortAlgorithms_h

#include "vpMultiGraphModel.h"

#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
struct SortVerticesByXPosDesc
{
  SortVerticesByXPosDesc(vpMultiGraphModel* graphModel,
                         vpMultiGraphModel::NodePositionType positionType) :
    GraphModel(graphModel), PositionType(positionType)
    {
    }

  bool operator ()(const vtkIdType& lhs, const vtkIdType& rhs)
    {
    double lx, ly, lz;
    double rx, ry, rz;

    this->GraphModel->GetNodePosition(this->PositionType, lhs, lx, ly, lz);
    this->GraphModel->GetNodePosition(this->PositionType, rhs, rx, ry, rz);

    return (lx > rx);
    }

  vtkSmartPointer<vpMultiGraphModel> GraphModel;
  vpMultiGraphModel::NodePositionType PositionType;
};


#endif // __vpGraphSortAlgorithms_h
