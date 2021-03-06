// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsEventStatus_h
#define __vsEventStatus_h

namespace vs
{
  enum EventStatus
    {
    UnverifiedEvent,
    VerifiedEvent,
    RejectedEvent
    };
}

typedef enum vs::EventStatus vsEventStatus;

#endif
