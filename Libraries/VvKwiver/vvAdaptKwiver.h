/*ckwg +5
 * Copyright 2017 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vvAdaptKwiver_h
#define __vvAdaptKwiver_h

#include <vvIqr.h>

#include <vgExport.h>

#include <memory>
#include <string>

namespace kwiver {
namespace vital {

class descriptor_request;
using descriptor_request_sptr = std::shared_ptr<descriptor_request>;

class track_descriptor;
using track_descriptor_sptr = std::shared_ptr<track_descriptor>;

class database_query;
using database_query_sptr = std::shared_ptr<database_query>;

class iqr_feedback;
using iqr_feedback_sptr = std::shared_ptr<iqr_feedback>;

class query_result;

}
}

template <typename A, typename B> class QHash;

namespace vvIqr
{

using ScoringClassifiers = QHash<long long, vvIqr::Classification>;

}

struct vvDescriptor;
struct vvProcessingRequest;
struct vvQueryInstance;
struct vvQueryResult;

extern VV_KWIVER_EXPORT vvDescriptor
fromKwiver(kwiver::vital::track_descriptor const&);

extern VV_KWIVER_EXPORT vvQueryResult
fromKwiver(kwiver::vital::query_result const&);

extern VV_KWIVER_EXPORT kwiver::vital::descriptor_request_sptr
toKwiver(vvProcessingRequest const&);

extern VV_KWIVER_EXPORT kwiver::vital::track_descriptor_sptr
toKwiver(vvDescriptor const&);

extern VV_KWIVER_EXPORT kwiver::vital::database_query_sptr
toKwiver(vvQueryInstance const&);

extern VV_KWIVER_EXPORT kwiver::vital::iqr_feedback_sptr
toKwiver(std::string const& queryId, vvIqr::ScoringClassifiers const&);

#endif
