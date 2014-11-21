
#include "vtkVgTrackModelCollection.h"

// VisGUI includes.
#include "vtkVgTrackModel.h"
#include "vtkVgTrackInfo.h"

#include <vtkVgTrack.h>


// VTK includes.
#include <vtkObjectFactory.h>

// C++ includes.
#include <algorithm>
#include <list>

vtkStandardNewMacro(vtkVgTrackModelCollection);

//-----------------------------------------------------------------------------
// Pimpl implementation.
class vtkVgTrackModelCollection::Internal
{
public:

  Internal()
    {
    this->InternalTrackModel = vtkVgTrackModel::SmartPtr::New();

    this->TrackModels.push_back(this->InternalTrackModel);
    }

  ~Internal()
    {
    this->TrackModels.clear();
    }

  void AddTracks(vtkVgTrackModel* otherTrackModel);

  vtkVgTrackModel::SmartPtr InternalTrackModel;

  std::list<vtkVgTrackModel::SmartPtr> TrackModels;

  typedef std::list<vtkVgTrackModel::SmartPtr>::iterator Itr;
  typedef std::list<vtkVgTrackModel::SmartPtr>::const_iterator ConstItr;
};

//-----------------------------------------------------------------------------
void vtkVgTrackModelCollection::Internal::AddTracks(vtkVgTrackModel* otherTrackModel)
{
  vtkVgTrack* track;
  this->InternalTrackModel->InitTrackTraversal();

  while ((track = this->InternalTrackModel->GetNextTrack().GetTrack()))
    {
    otherTrackModel->AddTrack(track);
    }
}

//-----------------------------------------------------------------------------
vtkVgTrackModelCollection::vtkVgTrackModelCollection() : vtkObject()
{
  this->Implementation = new Internal();
}

//-----------------------------------------------------------------------------
vtkVgTrackModelCollection::~vtkVgTrackModelCollection()
{
  delete this->Implementation; this->Implementation = 0;
}

//-----------------------------------------------------------------------------
void vtkVgTrackModelCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Number of track models in the collection: "
     << (this->Implementation->TrackModels.size() - 1) << "\n";
}

//-----------------------------------------------------------------------------
void vtkVgTrackModelCollection::AddTrack(vtkVgTrack* track)
{
  if (!track)
    {
    vtkErrorMacro("ERROR: Invalid (or NULL) track \n");
    return;
    }

  Internal::Itr itr = this->Implementation->TrackModels.begin();

  if (!track->GetPoints())
    {
    track->SetPoints(this->Implementation->InternalTrackModel->GetPoints());
    }
  else if (track->GetPoints() !=
           this->Implementation->InternalTrackModel->GetPoints())
    {
    vtkWarningMacro("ERROR: Points in incoming track does not match internal points \n");
    track->SetPoints(this->Implementation->InternalTrackModel->GetPoints());
    }

  for (; itr != this->Implementation->TrackModels.end(); ++itr)
    {
    (*itr)->AddTrack(track);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModelCollection::RemoveTrack(vtkIdType id)
{
  Internal::Itr itr = this->Implementation->TrackModels.begin();

  for (; itr != this->Implementation->TrackModels.end(); ++itr)
    {
    (*itr)->RemoveTrack(id);
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
vtkVgTrack* vtkVgTrackModelCollection::GetTrack(vtkIdType id)
{
  return this->Implementation->InternalTrackModel->GetTrack(id);
}

//-----------------------------------------------------------------------------
vtkVgTrackModel* vtkVgTrackModelCollection::GetInternalTrackModel()
{
  return this->Implementation->InternalTrackModel;
}

//-----------------------------------------------------------------------------
void vtkVgTrackModelCollection::AddTrackModel(vtkVgTrackModel* trackModel)
{
  if (!trackModel)
    {
    vtkErrorMacro("ERROR: Invalid (or NULL) track model \n");
    return;
    }

  // Check if the track model already exists.
  std::list<vtkVgTrackModel::SmartPtr>::iterator itr =
    std::find(this->Implementation->TrackModels.begin(),
              this->Implementation->TrackModels.end(),  trackModel);

  if (itr != this->Implementation->TrackModels.end())
    {
    return;
    }

  // Make sure that all track models with in this collection are sharing
  // same vtkPoints.
  if (trackModel->GetPoints() !=
      this->Implementation->InternalTrackModel->GetPoints())
    {
    vtkWarningMacro("WARNING: Points in incoming track model does not match internal points \n");

    trackModel->SetPoints(
      this->Implementation->InternalTrackModel->GetPoints());
    }

  this->Implementation->TrackModels.push_back(trackModel);

  // Add current tracks to the newly added track model.
  this->Implementation->AddTracks(trackModel);

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkVgTrackModelCollection::RemoveTrackModel(vtkVgTrackModel* trackModel)
{
  if (!trackModel)
    {
    vtkErrorMacro("ERROR: Invalid (or NULL) track model \n");
    return;
    }

  this->Implementation->TrackModels.remove(trackModel);

  this->Modified();
}


