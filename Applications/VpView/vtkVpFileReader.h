/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vtkVpFileReader_h
#define __vtkVpFileReader_h

#include "vtkVpReaderBase.h"

#include "vtkVgTimeStamp.h"

class vpEventConfig;
class vtkMatrix4x4;
class vtkVgActivityManager;
class vtkVgEventModel;
class vtkVgEvent;
class vtkVgTrackModel;
class vtkVgTrack;

class vtkVpFileReader : public vtkVpReaderBase
{
public:

  // Description:
  // Standard VTK functions.
  static vtkVpFileReader* New();
  vtkTypeMacro(vtkVpFileReader, vtkVpReaderBase);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual int ReadTracks(const char* filename,
                         enumTrackStorageModes trackStorageMode,
                         vtkMatrix4x4* latLonToWorld,
                         double* overrideColor = 0);

  virtual int ReadTrackTraits(const char* filename);
  virtual int ReadEvents(const char* filename);
  virtual int ReadEventLinks(const char* filename);
  virtual int ReadActivities(const char* filename);
  virtual int Reset();

  int CreateUserEvent(int type, int trackId1, int trackId2);

  vtkGetStringMacro(TracksFileName);
  vtkGetStringMacro(EventsFileName);
  vtkGetStringMacro(ActivitiesFileName);

  bool GetNextValidTrackFrame(vtkVgTrack* t, vtkIdType startIndex,
                              vtkVgTimeStamp& timeStamp);

  bool GetPrevValidTrackFrame(vtkVgTrack* t, vtkIdType startIndex,
                              vtkVgTimeStamp& timeStamp);


protected:
  // internally for convenience
  vtkSetStringMacro(TracksFileName);
  vtkSetStringMacro(EventsFileName);
  vtkSetStringMacro(ActivitiesFileName);

  // Description:
  // Constructor / Destructor.
  vtkVpFileReader();
  virtual ~vtkVpFileReader();

  char*        TracksFileName;
  char*        EventsFileName;
  char*        ActivitiesFileName;


private:
  vtkVpFileReader(const vtkVpFileReader&);  // Not implemented.
  void operator=(const vtkVpFileReader&);  // Not implemented.
};

#endif // __vtkVIDTKtReader_h
