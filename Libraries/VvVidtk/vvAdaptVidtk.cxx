/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vvAdaptVidtk.h"

#include <QList>

#include <tracking_data/image_object.h>
#include <tracking_data/track_state.h>
#include <tracking_data/tracking_keys.h>

#include <vvTrack.h>

//-----------------------------------------------------------------------------
QList<vvTrackState> vvAdapt(const vidtk::track_state& vs)
{
  QList<vvTrackState> stateList;
  vidtk::image_object* obj_sptr = vs.get_image_object();
  if (obj_sptr != nullptr)
    {
      {
      const vidtk::image_object& obj = *obj_sptr;

      vvTrackState state;

      // Set time
      if (vs.time_.has_time())
        {
        state.TimeStamp.Time = vs.time_.time();
        }
      if (vs.time_.has_frame_number())
        {
        state.TimeStamp.FrameNumber = vs.time_.frame_number();
        }

      // Set image point and box
      const auto& img_loc = obj_sptr->get_image_loc();
      const auto& bbox = obj_sptr->get_bbox();
      state.ImagePoint.X = img_loc[0];
      state.ImagePoint.Y = img_loc[1];
      state.ImageBox.TopLeft.X = bbox.min_x();
      state.ImageBox.TopLeft.Y = bbox.min_y();
      state.ImageBox.BottomRight.X = bbox.max_x();
      state.ImageBox.BottomRight.Y = bbox.max_y();

      // Set world location, if available
      double wlLat, wlLong;
      if (vs.latitude_longitude(wlLat, wlLong))
        {
        state.WorldLocation.GCS = 4269;
        state.WorldLocation.Latitude = wlLat;
        state.WorldLocation.Longitude = wlLong;
        }

      // Set image object, if available
      const auto& boundary = obj.get_boundary();
      if (boundary.num_sheets())
        {
        // Oh, dear... vgl_polygon uses int in its operator[], unsigned int for
        // the return value of num_sheets(), all to access a std::vector where
        // both of the above should be size_t!
        //
        // ...hence this silly cast, and silly use of int for the index counter
        // type, to avoid compiler warnings (it is safe from overflow, since an
        // overflow will make it < 0 and the loop will not execute)
        int numSheets = static_cast<int>(boundary.num_sheets());

        for (int j = 0; j < numSheets; ++j)
          {
          vvTrackState stateWithObject = state;
          typedef vgl_polygon<vidtk::image_object::float_type>::sheet_t Sheet;
          const Sheet& sheet = boundary[j];

          // Set image object
          for (size_t n = 0; n < sheet.size(); ++n)
            {
            vvImagePointF ipf(sheet[n].x(), sheet[n].y());
            stateWithObject.ImageObject.push_back(ipf);
            }

          // Append state
          stateList.append(stateWithObject);
          }
        }
      else
        {
        // Fall back on image box
        vvImageBoundingBox& box = state.ImageBox;
        state.ImageObject.push_back(
          vvImagePointF(box.TopLeft.X, box.TopLeft.Y));
        state.ImageObject.push_back(
          vvImagePointF(box.BottomRight.X, box.TopLeft.Y));
        state.ImageObject.push_back(
          vvImagePointF(box.BottomRight.X, box.BottomRight.Y));
        state.ImageObject.push_back(
          vvImagePointF(box.TopLeft.X, box.BottomRight.Y));

        // Append state
        stateList.append(state);
        }
      }
    }
  return stateList;
}
