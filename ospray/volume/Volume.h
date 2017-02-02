// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "common/Managed.h"

namespace ospray {

  /*! \brief A Volume is an abstraction for the concrete object which
    performs the volume sampling.

    The actual memory layout, dimensionality, and source of samples
    are unknown to this class.  Subclasses may implement structured
    volumes, unstructured volumes, radial basis functions, etc.  A
    type string specifies a particular concrete implementation to
    createInstance().  This type string must be registered either in
    OSPRay proper, or in a loaded module using OSP_REGISTER_VOLUME.
  */
  struct OSPRAY_SDK_INTERFACE Volume : public ManagedObject
  {
#if EXP_DATA_PARALLEL
    struct OSPRAY_SDK_INTERFACE DataParallelPiece : public RefCount {
      /*! world space bounding box of this piece. it is assumed that
          this covers all the space that rays should be integrating
          over; so _including_ any ghost cells if those are required
          to bridge gaps between neighboring blocks */
      box3f worldBounds;
      /*! pointer to the parent containing this piece. note this is
          intentionally not a ref to avoid cycles in the ref-graph */
      Volume     *parent;
      /*! the actual volume that contains that piece of the data. NULL
          if not on this node */
      Ref<Volume> actualData;
      /*! handle to the owning process:objectID, so we know whom to
          ask for this block if so required. note that a volume block
          may be stored on multiple nodes, and thus may have multiple
          owners */
      std::vector<ObjectHandle> owner;
    };
#endif

    //! Destructor.
    virtual ~Volume() = default;

    //! \brief Returns whether the volume is a data-distributed volume
    virtual bool isDataDistributed() const;

    //! A string description of this class.
    virtual std::string toString() const override;

    //! Create a volume container of the given type.
    static Volume *createInstance(const std::string &type);

    //! Allocate storage and populate the volume.
    virtual void commit() override;

    //! Copy voxels into the volume at the given index (non-zero return value
    //!  indicates success).
    virtual int setRegion(const void *source,
                          const vec3i &index,
                          const vec3i &count) = 0;

    //! Compute samples at the given world coordinates.
    virtual void computeSamples(float **results,
                                const vec3f *worldCoordinates,
                                const size_t &count);

    //! Update select editable parameters (allowed after the volume has been
    //! initially committed).
    virtual void updateEditableParameters();

  protected:

    //! Complete volume initialization (only on first commit).
    virtual void finish();
  };

/*! \brief Define a function to create an instance of the InternalClass
  associated with external_name.

 \internal The function generated by this macro is used to create an
  instance of a concrete subtype of an abstract base class.  This
  macro is needed since the subclass type may not be known to OSPRay
  at build time.  Rather, the subclass can be defined in an external
  module and registered with OSPRay using this macro.
*/
#define OSP_REGISTER_VOLUME(InternalClass, external_name) \
  OSP_REGISTER_OBJECT(Volume, volume, InternalClass, external_name)

} // ::ospray
