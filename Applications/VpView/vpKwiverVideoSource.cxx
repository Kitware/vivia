/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vpKwiverVideoSource.h"

#include "vpFileDataSource.h"

namespace kv = kwiver::vital;

//-----------------------------------------------------------------------------
vpKwiverVideoSource::vpKwiverVideoSource(vpFileDataSource* ds) :
  CurrentFrame(-1)
{
  const auto totalFrames = ds->getFileCount();
  for (int i = 0; i < totalFrames; ++i)
    {
    this->FramePaths.push_back(ds->getDataFile(i));
    }
}

//-----------------------------------------------------------------------------
vpKwiverVideoSource::~vpKwiverVideoSource()
{
}

//-----------------------------------------------------------------------------
void vpKwiverVideoSource::set_configuration(
  kwiver::vital::config_block_sptr config)
{
  this->Loader.set_configuration(config);
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::check_configuration(
  kwiver::vital::config_block_sptr config) const
{
  return this->Loader.check_configuration(config);
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
bool vpKwiverVideoSource::next_frame(kv::timestamp& ts, uint32_t)
{
  if (this->CurrentFrame < 0 || this->good())
    {
    ++this->CurrentFrame;
    }

  if (this->good())
    {
    ts = this->frame_timestamp();
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vpKwiverVideoSource::end_of_video() const
{
  return !this->good();
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
