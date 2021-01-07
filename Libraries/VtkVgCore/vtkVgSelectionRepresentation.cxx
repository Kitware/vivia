// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgSelectionRepresentation.h"
#include "vtkObjectFactory.h"

//-------------------------------------------------------------------------
vtkVgSelectionRepresentation::vtkVgSelectionRepresentation()
{
  this->InteractionState = vtkVgSelectionRepresentation::Outside;

  this->SelectionPoint[0] = -1.0;
  this->SelectionPoint[1] = -1.0;

  this->AnnotationVisibility = 0;

  this->MinimumSelectionMarkerSize = 10;
}

//-------------------------------------------------------------------------
vtkVgSelectionRepresentation::~vtkVgSelectionRepresentation()
{
}

//-------------------------------------------------------------------------
void vtkVgSelectionRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Number Of Items: " << this->GetNumberOfItems() << "\n";
  os << indent << "Item Number: " << this->GetNumberOfItems() << "\n";

  os << indent << "Selection Point: (" << this->SelectionPoint[0] << ","
     << this->SelectionPoint[1] << "}\n";

  os << indent << "Annotation Visibility: " << this->AnnotationVisibility << "\n";
}
