/*ckwg +5
 * Copyright 2018 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpKwiverVideoSource_h
#define __vpKwiverVideoSource_h

#include <arrows/vxl/image_io.h>

#include <vital/algo/video_input.h>

class vpFileDataSource;

class vpKwiverVideoSource : public kwiver::vital::algo::video_input
{
public:
  vpKwiverVideoSource(vpFileDataSource*);
  ~vpKwiverVideoSource();

  virtual void set_configuration(
    kwiver::vital::config_block_sptr) override;

  virtual bool check_configuration(
    kwiver::vital::config_block_sptr) const override;

  virtual void open(std::string) override;
  virtual void close() override;
  virtual bool good() const override;

  virtual bool seekable() const override { return true; }

  virtual bool next_frame(kwiver::vital::timestamp&, uint32_t) override;
  virtual bool seek_frame(kwiver::vital::timestamp&,
                          kwiver::vital::frame_id_t, uint32_t) override;
  virtual bool end_of_video() const override;
  virtual size_t num_frames() const override;

  virtual kwiver::vital::timestamp frame_timestamp() const override;
  virtual kwiver::vital::image_container_sptr frame_image() override;
  virtual kwiver::vital::metadata_vector frame_metadata() override;

  virtual kwiver::vital::metadata_map_sptr metadata_map() override;

protected:
  using frame_id_t = std::make_signed<size_t>::type;

  kwiver::arrows::vxl::image_io Loader;
  std::vector<std::string> FramePaths;
  frame_id_t CurrentFrame;

private:
  vpKwiverVideoSource(const vpKwiverVideoSource&) = delete;
  void operator=(const vpKwiverVideoSource&) = delete;
};

#endif
