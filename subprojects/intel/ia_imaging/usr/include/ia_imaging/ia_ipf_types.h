/*
 * Copyright (C) 2015 - 2017 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _IA_IPF_TYPES_H_
#define _IA_IPF_TYPES_H_

/** @file ia_ipf_types.h
 * This file contains data types for the Imaging Pipeline Framework (IPF).
 * This framework provides functions to construct and execute pipelines
 * consisting of so-called engines. Each pipeline stage executes one such
 * engine.
 * The engine structure defined in this file contains the interface that all
 * IPF compatible engines need to implement.
 * Each engine is expected to implement its own function to create an instance
 * of such an engine.
 */

#include <stdint.h>
#include <stdbool.h>

#include "ia_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The following macros define ranges for each of the IA imaging
 *  library's built-in IPF compatible modules. Each module
 *  will define its own buffer type identifiers for its own data
 *  types that are used for input, output or temporary (internal)
 *  buffers.
 */
#define IA_IPF_BUFFER_TYPES    0x00001000
#define IA_CP_BUFFER_TYPES     0x00002000
#define IA_FACE_BUFFER_TYPES   0x00003000
#define IA_REDEYE_BUFFER_TYPES 0x00004000
#define IA_ISP_BUFFER_TYPES    0x00005000
/** Externally defined engines can define their own buffer
 *  types starting at this number. */
#define IA_CUSTOM_BUFFER_TYPES 0x00100000

/**
 * Define specific buffer types below
*/
#define IA_IPF_BUFFER_TYPE_FRAME (IA_IPF_BUFFER_TYPES + 0)
#define IA_IPF_BUFFER_TYPE_INT32 (IA_IPF_BUFFER_TYPES + 1)
#define IA_FACE_BUFFER_TYPE_FACE_LIST (IA_FACE_BUFFER_TYPES + 0)

typedef struct ia_ipf_buffer_container_t ia_ipf_buffer_container;
typedef struct ia_ipf_buffer_container_descr_t ia_ipf_buffer_container_descr;

/** Container type.
 *  This enum contains possible types of containers.
 * Containers can either be a buffer or a buffer list.
 */
typedef enum {
    ia_ipf_container_type_buffer,
    ia_ipf_container_type_buffer_list,
}ia_ipf_container_type;

/** Buffer owner.
 *  This enum contains possible owners of buffers.
 *  Ownership is defined as the module that allocated the buffer.
 *  This means that the owner of the buffer is responsible for freeing
 *  the memory.
 */
typedef enum {
        ia_ipf_buffer_owner_user,      /**< Buffer is owned by the user of the IPF API */
        ia_ipf_buffer_owner_framework, /**< Buffer is owned by the IPF implementation */
        ia_ipf_buffer_owner_engine,    /**< Buffer is owned by the firmware engine */
} ia_ipf_buffer_owner;

/** Compute resources.
 *  This is used in a number of places where we need to know whether code runs
 *  on a particular resource.
 */
typedef enum {
        ia_ipf_resource_isp,  /**< The ISP is the compute resource */
        ia_ipf_resource_cpu,  /**< The CPU is the compute resource */
        ia_ipf_resource_num_resources,
} ia_ipf_resource;

/** IPF Buffer Descriptor.
 *  This structure describes various data buffers (for e.g. image frame) used by the IPF.
 */
typedef struct {
        int32_t                 type;     /**< Data type, module or user defined number */
        ia_ipf_buffer_owner     owner;    /**< Indicator of the buffer owner. */
        int32_t                 data_size;     /**<< Size, in bytes, of the memory */
        void                   *header;
        int32_t                 header_size;
} ia_ipf_buffer_descr;

/** IPF Buffer Descriptor List
 *  This structure contains a list of buffer descriptors.
 */
typedef struct {
        ia_ipf_buffer_container_descr *container_descrs;
        int32_t                        num_container_descrs;
} ia_ipf_buffer_descr_list;

/** IPF Buffer.
 *  This structure contains an individual buffer
 */
typedef struct {
        void                *data;   /**< Data pointer to raw pixel data */
} ia_ipf_buffer;

/** IPF Buffer list.
 *  This structure contains a list of buffers.
 */
typedef struct {
        ia_ipf_buffer_container *buffer_containers;     /**< Array of buffer containers */
        int32_t                  num_buffers; /**< Number of buffers in the array */
} ia_ipf_buffer_list;

/** IPF Buffer Container Descriptor List.
 *  This structure describes the buffer container descriptor used in the IPF framework.
 */
struct ia_ipf_buffer_container_descr_t{
  ia_ipf_container_type container_type; /**<< container type */
  union {
  ia_ipf_buffer_descr        buffer_descr;
  ia_ipf_buffer_descr_list   buffer_descr_list;
  }container;                           /**<< container descriptor */
};

/** IPF Buffer Container.
 *  This structure describes the buffer container used in the IPF framework.
 */
struct ia_ipf_buffer_container_t{
  ia_ipf_buffer_container_descr container_descr; /**<< container descriptor */
  union {
  ia_ipf_buffer        *buffer;
  ia_ipf_buffer_list   *buffer_list;
  }container;                                   /**<< container that holds a buffer or buffer list */
};

/** IPF Engine structure.
 *  This is the structure that each engine is required to implement in order to
 *  be compatbile with the IPF framework.
 *  Each engine is responsible for providing a function to create such an
 *  engine.
 *  Note that this describes an instance of an engine and each engine can be
 *  instantiated multiple times.
 *  Engines are free to allocate more memory than the size of this structure to
 *  hold internal state. This is why the destroy function in this structure
 *  must always be used to free the allocated memory; do not free this structure
 *  yourself.
 */
typedef struct ia_ipf_engine_t {
        const char *isp_fw_file;
        /**< Filename of the ISP firmware file. This will be NULL for non-ISP
             engines. This filename should not include a path. */

        const char *isp_fw_directory;
        /**< Directory that contains the FW file. Built-in engines will have
             this set to NULL in which case the IPF FW directory will be used.
             Custom engines should specify the path themselves. */

        ia_err     (*init)(struct ia_ipf_engine_t *me,
                           const ia_ipf_buffer_list *input_buffers,
                           ia_ipf_buffer_descr_list **output_buffer_descrs_list,
                           ia_ipf_buffer_descr_list **temp_buffer_descrs_list,
                           int32_t                   load[ia_ipf_resource_num_resources]);
        /**< Initialization function. This initializes this engine instance with
             a set of input parameters (buffers). This function will fill in the
             output_buffer_descrs_list, load and if the engine runs on the ISP,
             the temp_buffers_descrs_list should also be filled in. The engine
             framework will use this information to allocate all necessary
             buffers. */

        ia_err     (*isp_preprocess)(struct ia_ipf_engine_t *me,
                                     const ia_ipf_buffer_list *input_buffers,
                                     const ia_ipf_buffer_list *output_buffers,
                                     ia_ipf_buffer_list       *temp_buffers);
        /**< Pre-process arguments for ISP binary. ISP engines will fill
             in this function pointer. CPU engines will set this to NULL.
             This will convert input and output buffer pointers into the ISP
             internal format which is stored in temp_buffers.
             ISP engines need to implement this function because only the
             temp_buffers are passed as arguments to the ISP FW. */

        ia_err     (*isp_postprocess)(struct ia_ipf_engine_t *me,
                                      const ia_ipf_buffer_list *input_buffers,
                                      ia_ipf_buffer_list       *output_buffers,
                                      const ia_ipf_buffer_list *temp_buffers);
        /**< Post-process the ISP FW output, this step is optional.
             When implemented, this function will peform some type of
             post-processing on the ISP output before the CPU can use this
             output data. */

        ia_err     (*execute)(struct ia_ipf_engine_t *me,
                              const ia_ipf_buffer_list *input_buffers,
                              ia_ipf_buffer_list       *output_buffers,
                              ia_ipf_buffer_list       *temp_buffers);
        /**< Execute a CPU-only engine. ISP engines will be executed by the
             framework implementation using the acceleration API; ISP
             engines should set this pointer to NULL. */

        ia_err     (*cancel)(struct ia_ipf_engine_t *me,
                             ia_ipf_buffer_list *info);
        /**< Cancel a CPU-only engine if it's currently running. If the
             engine is not running when this function is called, this will
             be a no-op. */

        void      (*destroy)(struct ia_ipf_engine_t *me);
        /**< Destroy this engine instance. This will free any engine-owned
             internal state as well as this structure. */
} ia_ipf_engine;

#ifdef __cplusplus
}
#endif

#endif /* _IA_IPF_TYPES_H_ */
