/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vtkVgAreaPicker.h"

// VTK includes.
#include "vtkAreaPicker.h"
#include "vtkObjectFactory.h"
#include "vtkMapper.h"
#include "vtkAbstractVolumeMapper.h"
#include "vtkImageMapper3D.h"
#include "vtkAbstractMapper3D.h"
#include "vtkProp.h"
#include "vtkLODProp3D.h"
#include "vtkActor.h"
#include "vtkPropCollection.h"
#include "vtkImageSlice.h"
#include "vtkProp3DCollection.h"
#include "vtkAssemblyPath.h"
#include "vtkImageData.h"
#include "vtkVolume.h"
#include "vtkRenderer.h"
#include "vtkProperty.h"
#include "vtkCommand.h"
#include "vtkPlanes.h"
#include "vtkPlane.h"
#include "vtkPoints.h"
#include "vtkExtractSelectedFrustum.h"

vtkStandardNewMacro(vtkVgAreaPicker);

int vtkVgAreaPicker::PickProps(vtkRenderer* renderer)
{
  vtkProp* prop;
  int pickable;
  double bounds[6];

  //  Initialize picking process
  this->Initialize();
  this->Renderer = renderer;

  // Invoke start pick method if defined
  this->InvokeEvent(vtkCommand::StartPickEvent, NULL);

  if (renderer == NULL)
    {
    vtkErrorMacro(<< "Must specify renderer!");
    return 0;
    }

  //  Loop over all props.
  //
  vtkPropCollection* props;
  vtkProp* propCandidate;
  if (this->PickFromList)
    {
    props = this->GetPickList();
    }
  else
    {
    props = renderer->GetViewProps();
    }

  vtkAbstractMapper3D* mapper = NULL;
  vtkAssemblyPath* path;

  double mindist = VTK_DOUBLE_MAX;

  vtkCollectionSimpleIterator pit;
  for (props->InitTraversal(pit); (prop = props->GetNextProp(pit));)
    {
    for (prop->InitPathTraversal(); (path = prop->GetNextPath());)
      {
      propCandidate = path->GetLastNode()->GetViewProp();
      pickable = this->TypeDecipher(propCandidate, &mapper);

      //  If actor can be picked, see if it is within the pick frustum.
      if (pickable)
        {
        if (mapper)
          {
          double* bds = propCandidate->GetBounds();
          for (int i = 0; i < 6; i++)
            {
            bounds[i] = bds[i];
            }

          double dist;
          //cerr << "mapper ABFISECT" << endl;
          if (this->ABoxFrustumIsect(bounds, dist))
            {
            if (! this->Prop3Ds->IsItemPresent(prop))
              {
              this->Prop3Ds->AddItem(static_cast<vtkProp3D*>(prop));
              //cerr << "picked a mapper" << endl;
              if (dist < mindist) //new nearest, remember it
                {
                mindist = dist;
                this->SetPath(path);
                this->Mapper = mapper;
                vtkMapper* map1;
                vtkAbstractVolumeMapper* vmap;
                vtkImageMapper3D* imap;
                if ((map1 = vtkMapper::SafeDownCast(mapper)) != NULL)
                  {
                  this->DataSet = map1->GetInput();
                  this->Mapper = map1;
                  }
                else if ((vmap = vtkAbstractVolumeMapper::SafeDownCast(mapper)) != NULL)
                  {
                  this->DataSet = vmap->GetDataSetInput();
                  this->Mapper = vmap;
                  }
                else if ((imap = vtkImageMapper3D::SafeDownCast(mapper)) != NULL)
                  {
                  this->DataSet = imap->GetDataSetInput();
                  this->Mapper = imap;
                  }
                else
                  {
                  this->DataSet = NULL;
                  }
                }
              }
            }
          }//mapper
        }//pickable

      }//for all parts
    }//for all props

  int picked = 0;

  if (this->Path)
    {
    // Invoke pick method if one defined - prop goes first
    this->Path->GetFirstNode()->GetViewProp()->Pick();
    this->InvokeEvent(vtkCommand::PickEvent, NULL);
    picked = 1;
    }

  // Invoke end pick method if defined
  this->InvokeEvent(vtkCommand::EndPickEvent, NULL);

  return picked;
}
