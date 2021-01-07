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

#ifndef _IA_IPF_BUFFER_LIST_H_
#define _IA_IPF_BUFFER_LIST_H_

#include "ia_ipf_types.h"
#include "ia_ipf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IPF helper function
*/
ia_ipf_buffer_list *
ia_ipf_buffer_list_create_from_frame (ia_frame *inputframe);

/**
 * IPF helper function
*/
ia_err
ia_ipf_buffer_list_create_from_descr (const ia_ipf_buffer_descr_list *descrs_list, ia_ipf_buffer_list **buf_list);

/**
 * IPF helper function to create buffer list
*/
ia_ipf_buffer_list *
ia_ipf_buffer_list_create (const ia_ipf_buffer_descr_list *descrs_list);

/**
 * Helper function to free ipf buffer list
*/
void
ia_ipf_buffer_list_destroy (ia_ipf_buffer_list *buffer_list);

#ifdef __cplusplus
}
#endif

#endif /* _IA_IPF_BUFFER_LIST_H_ */

