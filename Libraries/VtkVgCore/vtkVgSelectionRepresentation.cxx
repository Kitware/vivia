/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
