// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// .NAME vtkVgVideoCache - cache a subset of a video stream.
// .SECTION Description
// Video streams are expressed as a series of frames (images) over time.
// The entire video stream is characterized with a whole extent defining
// a space-time range. Requests can be made of subsets of this video stream
// which are returned in a vtkImageData.
//
// Note that the cache contains the actual data. It knows the whole extent
// of the data, but maintains data representing a subset of the whole extent
// at potentially a different resolution.

#ifndef __vtkVgVideoCache_h
#define __vtlVideoManager_h

#include "vtkObject.h"
#include "vtkSmartPointer.h" // for memory safety

#include <vgExport.h>

class vtkImageData;
class vtkXMLImageDataReader;

class VTKVG_CORE_EXPORT vtkVgVideoCache : public vtkObject
{
public:
  static vtkVgVideoCache* New();
  vtkTypeMacro(vtkVgVideoCache, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the whole extent of the video stream. This is a 6-tuple
  // indicating x-y-z-time min and max range. The ranges are zero
  // offset, and time is expressed in frame numbers. The data values
  // are (xmin,xmax,ymin,ymax,tmin,tmax).
  vtkSetVector6Macro(WholeExtent, unsigned int);
  vtkGetVector6Macro(WholeExtent, unsigned int);

  // Description:
  // Specify the extent of the video cache. This is a 6-tuple
  // indicating x-y-time min and max range. The ranges are zero
  // offset, and time is expressed in frame numbers. The data values
  // are (xmin,xmax,ymin,ymax,tmin,tmax). Note that the extent should
  // lie within the WholeExtent (and is in the coordinate system of the
  // WholeExtent).
  vtkSetVector6Macro(Extent, unsigned int);
  vtkGetVector6Macro(Extent, unsigned int);

  // Description:
  // Return the resolution of this cache. This is the ratio of the actual
  // extent to the whole extent of the data. (Note that the minimium
  // resolution in the x-y directions is returned.)
  double GetResolution();

  // Description:
  // Return 1 if the cache contains the requested data, 0 otherwise.
  int ContainsRequest(unsigned int e[6]);

  // Description:
  // Given a request e[6], retrieve the data from the cache and place it into
  // the vtkImageData. Note that request e[6] should have been validated
  // previously with a ContainsRequest() invocation.
  void GetData(unsigned int e[6], vtkSmartPointer<vtkImageData>);

  // Description:
  // Given a request for data (the request e[6] is expressed in the WholeExtent
  // coordinate system), ask the cache's reader to load data asynchronously.
  // We expect the reader to throw an event when the cache is loaded. Note that
  // the response to this request is that the LoadData method will update the
  // cache with data that covers e[6] (it may be greater in extent).
  void RequestData(unsigned int e[6]);

  // Description:
  // This method loads data into the cache (typically in response to a
  // RequestData() invocation). As it does so, it defines the Extent and
  // DataExtent of the cache.
  void LoadData(unsigned int extent[6], vtkSmartPointer<vtkImageData> image);

  // Description:
  // This special method reads data and configures the cache. This typically
  // is used for the low-res, full extent cache (Cache 0, and e[6] ==
  // WholeExtent[6]).  The WholeExtent and Extent are set (to the same value)
  // on read if initializeCache is set.
  void LoadCacheFromFile(char* file, int wholeExtentFlag, unsigned int e[6]);

protected:
  vtkVgVideoCache();
  virtual ~vtkVgVideoCache();

private:
  vtkVgVideoCache(const vtkVgVideoCache&);  // Not implemented.
  void operator=(const vtkVgVideoCache&);    // Not implemented.

  unsigned int WholeExtent[6];
  unsigned int Extent[6];
  int          HasData;

  vtkSmartPointer<vtkImageData> ImageData;
  double Resolution;

  // Various readers
  vtkSmartPointer<vtkXMLImageDataReader> CacheReader;

  // Convenience functions
  void ConvertWholeExtentsToCacheExtents(unsigned int wE[6], unsigned int cE[6]);

};

#endif // __vtkVgVideoCache_h
