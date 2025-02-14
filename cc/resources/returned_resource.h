// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RESOURCES_RETURNED_RESOURCE_H_
#define CC_RESOURCES_RETURNED_RESOURCE_H_

#include <vector>

#include "cc/base/cc_export.h"
#include "cc/base/resource_id.h"
#include "gpu/command_buffer/common/sync_token.h"

namespace cc {

// A ReturnedResource is a struct passed along to a child compositor from a
// parent compositor that corresponds to a TransferableResource that was
// first passed to the parent compositor.
struct CC_EXPORT ReturnedResource {
  ReturnedResource() : id(0), count(0), lost(false) {}

  bool operator==(const ReturnedResource& other) const {
    return id == other.id && sync_token == other.sync_token &&
           count == other.count && lost == other.lost;
  }

  bool operator!=(const ReturnedResource& other) const {
    return !(*this == other);
  }

  // |id| is an identifier generated by the child compositor that uniquely
  // identifies a resource. This is the same ID space as TransferableResource.
  ResourceId id;

  // A |sync_token| is an identifier for a point in the parent compositor's
  // command buffer. The child compositor then issues a WaitSyncPointCHROMIUM
  // command with this |sync_token| as a parameter into its own command buffer.
  // This ensures that uses of the resource submitted by the parent compositor
  // are executed before commands submitted by the child.
  gpu::SyncToken sync_token;

  // |count| is a reference count for this resource. A resource may be used
  // by mulitple compositor frames submitted to the parent compositor. |count|
  // is the number of references being returned back to the child compositor.
  int count;

  // If the resource is lost, then the returner cannot give a sync point for it,
  // and so it has taken ownership of the resource. The receiver cannot do
  // anything with the resource except delete it.
  bool lost;
};

typedef std::vector<ReturnedResource> ReturnedResourceArray;

}  // namespace cc

#endif  // CC_RESOURCES_RETURNED_RESOURCE_H_
