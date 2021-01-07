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

#ifndef _IA_IPF_BUFFER_CONTAINER_H_
#define _IA_IPF_BUFFER_CONTAINER_H_

#include "ia_ipf_types.h"
#include "ia_ipf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ia_ipf_container_alloc_header,
    ia_ipf_container_alloc_data,
} ia_ipf_container_alloc;

/**
 * IPF helper to populate header and data buffer container fields
*/
void
ia_ipf_buffer_container_populate(ia_ipf_buffer_container *buffer_container, void *header, void *data);

/**
 * Helper function to get a buffer container from a frame. This function allocates
 * the container that has to be freed by the user
 */
void
ia_ipf_bufffer_container_from_frame (ia_ipf_buffer_container *buffer_container, ia_frame *frame);

/**
 * Helper function to get a ia_frame from an ia_ipf_buffer
 */
ia_frame *
ia_ipf_buffer_container_to_frame (ia_ipf_buffer_container *buffer_container);

/*
 * Helper function to allocate memory for header and data field
*/
void
ia_ipf_buffer_container_init (ia_ipf_buffer_container *buffer_container);

/*
 * Helper function to deallocate memfory for header and data field
*/
void
ia_ipf_buffer_container_deinit (ia_ipf_buffer_container *buffer_container);

#ifdef __cplusplus
}
#endif

#endif /* _IA_IPF_BUFFER_CONTAINER_H_ */

