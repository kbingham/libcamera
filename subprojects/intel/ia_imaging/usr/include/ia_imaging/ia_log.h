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

#include "ia_types.h"

#ifndef _IA_LOG_H_
#define _IA_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __func__
#define __func__ __FUNCTION__
#endif

/*!
 * \brief Macro for not logging.
 */
#define IA_NOLOG(...)           ((void)0)

/*!
 * \brief Macro wrapper for ia_log function.
 */
#define IA_LOG(level, fmt, ...) ((void)ia_log(level, fmt, ## __VA_ARGS__))

/*!
 * \brief Logging levels.
 */
typedef enum
{
    ia_log_error,
    ia_log_debug,
    ia_log_info,
} ia_log_level;

/*!
 * \brief Initializes ia_log library with external function pointers for logging.
 * If this function is not called, logging will be directed to stdout and stderr.
 *
 * \param[in] a_env_ptr Structure containing the addresses to logging functions.
 *                      If NULL is passed as function pointer, there won't be log prints.
 * \return              Error code.
 */
LIBEXPORT ia_err
ia_log_init(ia_env *a_env_ptr);

/*!
 * \brief Prints formatted string.
 *
 *
 * \param[in] level Log level.
 * \param[in] fmt   Format string.
 * \param[in] ...   Variables to be printed out as defined by the format string.
 */
LIBEXPORT void
ia_log(ia_log_level level, const char *fmt, ...);

/*!
 * \brief Prints formatted string with interpreted error code.
 *
 *
 * \param[in]     error               Error code.
 * \return                            Pointer to all Error string.
 */
LIBEXPORT const char *
ia_log_strerror(ia_err error);

/*!
 * \brief De-initialization of ia_log library.
 * Must be called if ia_log_init() was called.
 */
LIBEXPORT void
ia_log_deinit();

#ifdef __cplusplus
}
#endif

#endif /* _IA_LOG_H_ */
