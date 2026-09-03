// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vpKwiverVideoSource.h"

#include "vpFileDataSource.h"

#include <qtStlUtil.h>

#include <QStringList>

namespace kv = kwiver::vital;

//-----------------------------------------------------------------------------
vpKwiverVideoSource::vpKwiverVideoSource(vpFileDataSource* ds) :
  CurrentFrame(-1)
{
  for (const auto& f : ds->frameNames())
    {
    this->FramePaths.push_back(stdString(f));
    }
}

//-----------------------------------------------------------------------------
vpKwiverVideoSource::~vpKwiverVideoSource()
{
}

//-----------------------------------------------------------------------------
void vpKwiverVideoSource::set_configuration(
  kv::config_block_sptr)
{
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::check_configuration(
  kv::config_block_sptr) const
{
  return true;
}

//-----------------------------------------------------------------------------
void vpKwiverVideoSource::open(std::string)
{
}

//-----------------------------------------------------------------------------
void vpKwiverVideoSource::close()
{
  this->CurrentFrame = -1;
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::good() const
{
  const auto totalFrames = static_cast<frame_id_t>(this->FramePaths.size());
  return (this->CurrentFrame >= 0 && this->CurrentFrame < totalFrames);
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::next_frame(kv::time_usec_t)
{
  if (this->CurrentFrame < 0 || this->good())
    {
    ++this->CurrentFrame;
    }

  return this->good();
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::seek_frame(
  kv::timestamp::frame_t frame, kv::time_usec_t)
{
  if (frame < 0 || frame > static_cast<frame_id_t>(this->num_frames()))
    {
    return false;
    }

  this->CurrentFrame = static_cast<frame_id_t>(frame);

  return true;
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::seek_time(kv::timestamp::time_t, kv::time_usec_t)
{
  return false;
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::end_of_video() const
{
  return !this->good();
}

//-----------------------------------------------------------------------------
size_t vpKwiverVideoSource::num_frames() const
{
  return this->FramePaths.size();
}

//-----------------------------------------------------------------------------
kv::timestamp vpKwiverVideoSource::frame_timestamp() const
{
  auto ts = kv::timestamp{};
  ts.set_frame(static_cast<kv::frame_id_t>(this->CurrentFrame));

  return ts;
}

//-----------------------------------------------------------------------------
kv::image_container_sptr vpKwiverVideoSource::frame_image()
{
  if (!this->good())
    {
    return nullptr;
    }

  return this->Loader.load(
    this->FramePaths[static_cast<size_t>(this->CurrentFrame)]);
}

//-----------------------------------------------------------------------------
kv::metadata_vector vpKwiverVideoSource::frame_metadata()
{
  return {};
}
