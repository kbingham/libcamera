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

#ifndef _IA_MONTAGE_H_
#define _IA_MONTAGE_H_

/** @file ia_montage.h
 * This file declares structures and APIs of image montage engine.
 */

#include "ia_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Image montage state. This is the handle for IM engine.
 */
typedef struct {
    ia_frame *output_image;                             /**< Allocated and managed internally, result image will be stored here. */
} ia_montage_state;


/** @brief Parameters for ia montage engine.
 * Used by ia_montage_set_parameter() and ia_montage_get_parameter() to indicate runtime engine configuration.
 */
typedef struct {
    int32_t search_region_margin_percentage;            /**< Indicates how much enlarged area to be considered when it crops the object image patch from sub image. Default is 20. */
} ia_montage_parameters;

/** @brief Initialize Image Montage engine
 *
 *  @param env [IN] Platform environment parameters.
 *
 * This function instantiates and initializes the Image Montage Engine.
 */
LIBEXPORT
ia_montage_state *
ia_montage_init(const ia_env *env);

/** @brief Destroy Image Montage Engine.
 */
LIBEXPORT
void
ia_montage_uninit(ia_montage_state *state);

/** @brief Initialize existing Image Montage engine.
 */
LIBEXPORT
void
ia_montage_reinit(ia_montage_state *state);

/** @brief Get version information of the Image Montage Engine.
 */
LIBEXPORT
ia_version
ia_montage_get_version(void);

/** @brief Set the parameters.
 *
 *  @param ms       [IN] Image Montage engine context.
 *  @param params   [IN] the parameter struct.
 *
 * Set ia_montage parameters to the engine context. The params is supposed to be filled by ia_montage_get_parameters().
 */
LIBEXPORT
ia_err
ia_montage_set_parameter(ia_montage_state *ms, const ia_montage_parameters *params);

/** @brief Get the parameters.
 *
 *  @param ms       [IN] Image Montage engine context.
 *  @param params   [IN/OUT] the parameter struct.
 *
 * Get ia_montage parameters from the engine context. The params must be allocated before calling this function. The members
 * would be filled after it returns.
 */
LIBEXPORT
ia_err
ia_montage_get_parameter(ia_montage_state *ms, ia_montage_parameters *params);

/** @brief Set the main image.
 *
 *  @param ms          [IN] Image Montage engine context.
 *  @param main_frame  [IN] Main image.
 *  @param objects     [IN] an array of the objects coordinates.
 *  @param num_objects [IN] the number of objects.
 *
 * The main image is labelled as the "background" for Image Montage.
 * Various objects are necessary to consider the overlapping between each object. Objects could be faces for general use-cases.
 * It is required to call this apposed to ia_montage_set_sub_images(), prior to composing.
 */

LIBEXPORT
ia_err
ia_montage_set_main_image(ia_montage_state *ms, const ia_frame *main_frame, ia_rectangle* objects, int32_t num_objects);

/** @brief Set the sub image where the object patches come from.
 *
 *  @param ms          [IN] Image Montage engine context.
 *  @param sub_frame   [IN] Sub image.
 *  @param objects     [IN] the coordinates of objects.
 *  @param num_objects [IN] the number of objects.
 *
 * The main image is labelled as the "background" for Image Montage.
 * Various objects are necessary to consider the overlapping between each object. Objects could be faces for general use-cases.
 * It is required to call this apposed to ia_montage_set_sub_images(), prior to composing.
 * It is fine to call this before than ia_montage_set_main_image().
 */
LIBEXPORT
ia_err
ia_montage_set_sub_image(ia_montage_state *ms, const ia_frame *sub_frame, ia_rectangle* objects, int32_t num_objects);

/** @brief Compose the image montage, cropping from the sub image, pasting to the main image.
 *
 *  @param ms           [IN] Image Montage engine context.
 *  @param idx_on_main  [IN] the index of the object given along with main image.
 *  @param idx_on_sub   [IN] the index of the object given along with sub image.
 *
 * This is to combine two images(main, sub), cropping an object image patch placed on 'idx_on_sub'th place of sub objects array,
 * overlaying it onto the place of 'idx_on_image' th of main objects array.
 * The composed image would be in the engine context.
 * Every time this function is called, the result image will be newly composed from main/sub images and the previous image will gone.
 * You should call ia_montage_set_main_image() again with the result image, in order to paste a new object on the result image,
 *
 */
LIBEXPORT
ia_err
ia_montage_compose(ia_montage_state *ms, int32_t idx_on_main, int32_t idx_on_sub);




#ifdef __cplusplus
}
#endif


#endif /* _IA_MONTAGE_H_ */
