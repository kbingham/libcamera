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

/*!
 * \file ia_coordinate.h
 * \brief Definitions of IA imaging coordinate system and conversion functions.
 *
 * IA Imaging algorithms use relative coordinate system where top left corner
 * is (0,0) and bottom right corner is (8192,8192).
 *
 * Coordinate conversions can happen between arbitrary coordinate systems.
 *
 * Width and height of coordinate system must be > 0.
 *
 * Zoom factor is not considered in coordinate conversions but it should be
 * calculated when defining the target coordinate system.
 *
 * For example:
 *
 *      |-------------------------------------------|
 *      |(0,0)                                      |
 *      |                                           |
 *      |    (1800,1700)                            |
 *      |        |--------------|                   |
 *      |        |(-100,-100)   |                   |
 *      |        |              |                   |
 *      |        |     (100,100)|                   |
 *      |        |--------------|                   |
 *      |                   (5500,4000)             |
 *      |                                           |
 *      |                                           |
 *      |                                           |
 *      |                                           |
 *      |                                (8192,8192)|
 *      |-------------------------------------------|
 *
 *
 * There is an algorithm which works on cropped area with own coordinate system A
 * [(-100,100), (100.100)] inside another coordinate system B [(0,0), (8192,8192)].
 *
 * To convert results from coordinate system A to B, one must define source and target
 * coordinate systems:
 * Source system is coordinate system A.
 * src_system: [(-100,100), (100.100)]
 *
 * Target system defines the cropped rectangle from inside coordinate system B.
 * trg_system: [(1800,1700), (5500,4000)]
 *
 */


#ifndef IA_COORDINATE_H_
#define IA_COORDINATE_H_

#include "ia_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Definitions of IA imaging coordinate system. */
#define IA_COORDINATE_TOP 0
#define IA_COORDINATE_LEFT 0
#define IA_COORDINATE_BOTTOM 8192
#define IA_COORDINATE_RIGHT 8192
#define IA_COORDINATE_WIDTH (IA_COORDINATE_RIGHT-IA_COORDINATE_LEFT)
#define IA_COORDINATE_HEIGHT (IA_COORDINATE_BOTTOM-IA_COORDINATE_TOP)

/*!
 * \brief Coordinate system.
 * Defines the coordinate space boundaries.
 */
typedef struct
{
    long top;    /*!< Top coordinate value in the coordinate system. */
    long left;   /*!< Left coordinate value in the coordinate system. */
    long bottom; /*!< Bottom coordinate value in the coordinate system. */
    long right;  /*!< Right coordinate value in the coordinate system. */
} ia_coordinate_system;

/*!
 * \brief Convert coordinate from source coordinate space to target coordinate space.
 * \param[in] src_system     Source coordinate system boundaries.
 * \param[in] trg_system     Target coordinate system boundaries.
 * \param[in] src_coordinate Coordinate values in source coordinate system.
 * \return                   Target coordinate converted from source coordinate.
 */
LIBEXPORT ia_coordinate
ia_coordinate_convert(
    const ia_coordinate_system *src_system,
    const ia_coordinate_system *trg_system,
    const ia_coordinate src_coordinate);

/*!
*  Converts a rectangle from one coordinate system to another.
*
*  \param [in]  a_src_system_ptr   The source coordinate system.
*  \param [in]  a_src_rect_ptr     The source rectangle.
*  \param [in]  a_tgt_system_ptr   The target coordinate system.
*  \param [out] a_tgt_rect_ptr     The calculated target rectangle.
*/
LIBEXPORT void
ia_coordinate_convert_rect(
    const ia_coordinate_system* a_src_system_ptr,
    const ia_rectangle* a_src_rect_ptr,
    const ia_coordinate_system* a_tgt_system_ptr,
    ia_rectangle* a_tgt_rect_ptr);

/*!
 * \brief Convert face coordinates to target coordinate system.
 * \param[in,out] face_state Structure containing face information from face tracker.
 * \param[in]     src_system Source coordinate system boundaries.
 * \param[in]     trg_system Target coordinate system boundaries.
 */
LIBEXPORT void
ia_coordinate_convert_faces(
    const ia_coordinate_system *src_system,
    const ia_coordinate_system *trg_system);

#ifdef __cplusplus
}
#endif


#endif /* IA_COORDINATE_H_ */

