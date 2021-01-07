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

#ifndef _IA_IPF_H_
#define _IA_IPF_H_

/** @file ia_ipf.h
 *
*/

#include <stdarg.h>

#include "ia_types.h"
#include "ia_ipf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pipe Line Header Information
*/

/**
 * unique identifier for a buffer in the pipe line
*/
typedef struct {
    ia_ipf_engine      *engine;         /**< */
    int32_t             index;          /**< output index : for a provider the index ranges {0,M-1} for M outputs */
    int32_t             nesting_index;  /**< output index to accommodates nesting levels for arrays */
    ia_ipf_buffer_owner owner;          /**< Indicator of the buffer owner - The owner can be a user app */
} ia_ipf_pipe_engine_input;

/**
 * container structure for identifiers of one or multiple buffers involved in the pipe line
*/
typedef struct {
    ia_ipf_pipe_engine_input *connection;
    int32_t                   num_connections;
} ia_ipf_pipe_engine_input_container;

/**
 * unique identifier for a buffer in the pipe line
*/
typedef struct {
    int32_t     index;              /**< output index : for a provider the index ranges {0,M-1} for M outputs */
    int32_t     nesting_index;      /**< output index to accommodates nesting levels for arrays */
    int32_t     num_users;
    int32_t     time_to_live;
} ia_ipf_pipe_engine_output;

/**
 * container structure for identifiers of one or multiple buffers involved in the pipe line
*/
typedef struct {
    ia_ipf_pipe_engine_output *connection;
    int32_t                    num_connections;
} ia_ipf_pipe_engine_output_container;

/**
 * Enum for states for io conn in the pipe line
*/
typedef enum {
    ia_ipf_pipe_io_state_uninit,
    ia_ipf_pipe_io_state_pipe_io,
    ia_ipf_pipe_io_state_buf_list,
    ia_ipf_pipe_io_state_eng_init,
    ia_ipf_pipe_io_state_eng_execute,
    ia_ipf_pipe_io_state_eng_destroy,
    ia_ipf_pipe_io_state_buf_list_dealloc,
    ia_ipf_pipe_io_state_pipe_io_dealloc,
} ia_ipf_pipe_io_state;

/**
 * Enum for states of the pipe line
*/
typedef enum {
    ia_ipf_pipe_state_uninit,
    ia_ipf_pipe_state_created,
    ia_ipf_pipe_state_finalized,
    ia_ipf_pipe_state_processed,
    ia_ipf_pipe_state_eng_init,
    ia_ipf_pipe_state_eng_init_done,
    ia_ipf_pipe_state_eng_exec,
    ia_ipf_pipe_state_eng_exec_done,
    ia_ipf_pipe_state_destroy,
} ia_ipf_pipe_state;

/**
 * Declare IPF pipeline structure
*/
typedef struct ia_ipf_pipe_t ia_ipf_pipe;

/**
 * creation routine for a pipe line
*/
ia_err
ia_ipf_pipe_create ( const char *name, int num_engines, ia_ipf_pipe **pipe);

/**
 * add engine to a pipe line
 * inputs:
 *      1. pipe line
 *      2. engine to be added
 *      3. container structure for buffer identifiers
 *          : this is sufficient to specify all input buffer connectivity for the engine
 * outputs:
 *      engine_id
 *        : every engine on addition to the pipeline is assigned a unique engine ID which serially increments from 1
 *  return:
 *        error value
 */
ia_err
ia_ipf_pipe_add_engine (ia_ipf_pipe *pipe, ia_ipf_engine *engine, ia_ipf_pipe_engine_input_container *inputs);

/**
 * set user input to a pipe line
 *  inputs:
 *      1. pipe line
 *      2. container structure for buffer identifiers
 *          : this lists all the buffer identifiers that exists in the pipeline which are provided by the user
 *              application as the input to the pipe line
 *        3. container structure for input buffers
 *            : this specifies the actual input buffer with input data that is the user input to the pipeline that will be
 *                linked to the unique buffer identifiers that exists in the pipeline
 * return:
 *      error value
*/
ia_err
ia_ipf_pipe_set_input (ia_ipf_pipe *pipe, ia_ipf_engine *engine,
                       ia_ipf_buffer_list *output_buffers);

/**
 * Init engines in the pipeline
*/
void
ia_ipf_pipe_init_engines (ia_ipf_pipe *pipe);

/**
 * execute the pipe line
 * inputs:
 *      1. pipe line
 * outputs:
 *      1. output type (void **)
 *    return:
 *        error value
*/
ia_err
ia_ipf_pipe_execute_engines (ia_ipf_pipe *pipe,
                             ia_ipf_buffer_list **output_buffers);

/**
 * destroy the pipe line
 * inputs:
 *       1. pipe line
 * return:
 *      error value
*/
ia_err
ia_ipf_pipe_destroy (ia_ipf_pipe *pipe );

/**
 * IPF helper - create an identifier for a buffer
*/
ia_err
ia_ipf_pipe_engine_input_container_create (ia_ipf_pipe_engine_input_container **engine_input_container,
                                         ia_ipf_engine *engine,
                                         int32_t src_index, int32_t src_subindex,
                                         int32_t dest_index, int32_t dest_subindex,
                                         int32_t num_connects);

/**
 * IPF helper - adds a identifier for a buffer to container
*/
ia_err
ia_ipf_pipe_engine_input_container_add (ia_ipf_pipe_engine_input_container *engine_input_container,
                                      ia_ipf_engine *engine,
                                      int32_t src_index, int32_t src_subindex,
                                      int32_t dest_index, int32_t dest_subindex);




#ifdef __cplusplus
}
#endif

#endif /* _IA_IPF_H_ */

