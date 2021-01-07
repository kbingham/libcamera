/*
 * Copyright 2012-2017 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _IA_FACE_H_
#define _IA_FACE_H_

/** @file ia_face.h
 * This file declares structures and APIs of face related computer vision engines.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ia_types.h contains generic data types such as coordinate, rectangle etc */
#include "ia_types.h"

/** @brief Representing eyes.
 * This structure represents the eye position and the blink state.
 */
typedef struct {
    ia_coordinate position;                  /**< Position of the eye. */
    int32_t       blink_confidence;          /**< Blink status in [0..100] range where 0 means wide open eye and 100 means fully closed eye.
                                                  negative integer means blink result is invalid (based on invalid ED position result) */
} ia_eye;

/** @brief Representing 3 dimensional point.
 */
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} ia_face_3d_coordinate;

/** @brief Representing the result of Head Post Tracker.
 */
typedef struct {
    int32_t               processing_state;     /**< Processing state. 0 : Internal preparation stage where result values are not valid. 1 : Initialized state. 2 : tracking state */
    int32_t               focal_length;         /**< Focal length. */
    ia_face_3d_coordinate rotation;             /**< Rotation (in degree) movement of the head around x, y, z axis */
    ia_face_3d_coordinate translation;          /**< Translation movement of the head along x, y, z axis. */
    ia_face_3d_coordinate head_hexahedron[8];   /**< Each vertex position of hexahedron being tracked. */
    ia_face_3d_coordinate face_features[4];     /**< Face feature points (left eye, right eye, mouse left, mouse right) */
    ia_coordinate         head_hexahedron_2d[8];/**< 2D projected points of head hexahedron. */
    ia_coordinate         face_features_2d[4];  /**< 2D projected points of face feature points. */
} ia_face_headpose;

/** @brief Representing the face feature data for Face Recognition.
 */
typedef struct {
    uint32_t       id;                       /**< Unique id of the face feature data. */
    int32_t        data_size;                /**< Size of the face feature data in bytes. */
    int32_t        person_id;                /**< Unique id of the person associated with the face feature data. This MUST be positive. -1 means unknown person. */
    uint32_t       time_stamp;               /**< Time stamp when the face feature was generated. */
    int32_t        condition;                /**< Environmental information of the face. Currently not used. */
    int32_t        checksum;                 /**< Checksum of the face feature data. */
    int32_t        version;                  /**< Face recognizer version. */
    uint8_t       *data;                     /**< Face feature data. */
} ia_face_feature;

/** @brief Representing faces.
 * This structure represents all information of a face supported by this engine library.
 */
typedef struct {
    ia_rectangle   face_area;                /**< Bounding box of the face in the coordination system where (0,0) indicates left-top position. */
    int32_t        rip_angle;                /**< RIP (rotation in plane) angle in degrees. */
    int32_t        rop_angle;                /**< ROP (rotation out of plane) angle in degrees. */
    int32_t        tracking_id;              /**< Tracking id of the face. */
    int32_t        confidence;               /**< Confidence in face detection result. */
    int32_t        person_id;                /**< Person id (typically positive number) of the face. Filled after face recognition. -1 if not recognized. */
    int32_t        similarity;               /**< Similarity value between this face and the face in the database which was recognized as the person. */
    int32_t        best_ratio;               /**< Indicates the ratio between the most similar face and the 2nd similar face. */
    int32_t        face_condition;           /**< Indicates the light condition of the face. Not used at this moment. */
    int32_t        smile_state;              /**< Smile state of the face. -1 : invalid info (based on invalid ED position result)
                                                  0 : Non-smile, 1 : Smile, 2 : Start of the smile. Auxiliary variable for smile shutter. */
    int32_t        smile_score;              /**< Smile score in the range of [0..100] where 0 is non-smile state and 100 is full smile state. */
    ia_coordinate  mouth;                    /**< Mid-point of the mouth. */
    ia_eye         left_eye;                 /**< Left eye */
    ia_eye         right_eye;                /**< Right eye */
    int32_t        eye_validity;             /**< Indicates whether a face was processed to get eye positions */
    int32_t        skin_type_dark_likelihood;/**< Likelihood of skin type being dark. Bright skin likelihood = 100 - dark_skin_type_likelihood */
    int32_t        skin_type_validity;       /**< Validity of the Skin Likelihood */
} ia_face;

/** @brief Face state data.
 * This structure is used to hold inputs and outputs of functions and also works as a handle of the engine instance.
 */
typedef struct {
    int32_t        num_faces;                /**< Number of faces in the recently processed input frame. */
    ia_face       *faces;                    /**< Array of face information. */
} ia_face_state;

/** @brief Version of face engines
 * Represents versions of each face engine.
 */
typedef struct {
    ia_version detection_version;         /**< the Face Detection version */
    ia_version recognition_version;       /**< the Face Recognition version */
    ia_version smile_version;             /**< the Smile Detection version */
    ia_version blink_version;             /**< the Blink Detection version */
} ia_face_version;

/** @brief Parameters for face engine.
 * Used by ia_face_set_parameters() and ia_face_get_parameters() to indicate runtime engine configuration.
 */
typedef struct {
    int32_t max_num_faces;                   /**< Maximum number of detectable faces in one frame. Default is 32. */
    int32_t min_face_size;                   /**< Minimum size of face to detect. Default is 22. */
    int32_t rip_coverage;                    /**< RIP (rotation in plane) coverage. */
    int32_t rop_coverage;                    /**< ROP (rotation out of plane) coverage. */
    int32_t max_computable_pixels;           /**< Not for general usage. */
    int32_t smile_sensitivity;               /**< The smile sensitve level. Not for general usage. */
    int32_t smile_threshold;                 /**< The smile threshold. Not for general usage. */
} ia_face_parameters;

/** @brief Initialize face engines.
 *
 *  @param env [IN] platform environment parameters.
 *  @return face state data that will be used to hold results of subsequent API calls. Also works as a handle of the engines.
 *
 *  This function will instantiate and initialize all face engines.
 */
LIBEXPORT
ia_face_state *
ia_face_init(const ia_env *env);

/** @brief Destroy face engines.
 *
 *  @param fs [IN] face state data associated with the engine which was created by ia_face_init().
 *
 *  This funciton destroys face engines and deallocate all internal memory including ia_face_state struct itself.
 */
LIBEXPORT
void
ia_face_uninit(ia_face_state *fs);

/** @brief Re-initialize existing face engines.
 *
 *  @param fs [IN] face state data associated with the engine which was created by ia_face_init().
 *
 *  This function initializes all face engines. After this call, all internal state will be initialized to the same state as ia_face_init() is called.
 */
LIBEXPORT
void
ia_face_reinit(ia_face_state *fs);


/** @brief Reset all the contexts and results in the existing face engines.
 *
 *  @param fs [IN] face state data associated with the engine which was created by ia_face_init().
 *
 *  This function reset all the detected/recognized contexts. Note that the FR database will not be initialized.
 */
LIBEXPORT
void
ia_face_clear_result(ia_face_state *fs);

/**
 *  @brief Get current version of face engines.
 *
 *  @return a struct that contains version information of each face engine.
 */
LIBEXPORT
ia_face_version
ia_face_get_versions(void);

/** @brief Set face parameterss.
 *
 *  @param fs [IN] face state data.
 *  @param params [IN] parameters for face engines.
 *
 *  This function is used to configure face engines. Please see the description of ia_face_parameters structure for the detail.
 */
LIBEXPORT
ia_err
ia_face_set_parameters(ia_face_state *fs, const ia_face_parameters *params);

/** @brief Get current face parameters.
 *
 *  @param fs [IN] face state data.
 *  @param params [OUT] a buffer that will hold face parameters.
 *
 */
LIBEXPORT
ia_err
ia_face_get_parameters(ia_face_state *fs, ia_face_parameters *params);

/** @brief Set the debug message level.
 */
LIBEXPORT
void
ia_face_set_debug_level(ia_face_state *fs, int32_t level);

/** @brief Set acceleration APIs
 *
 *  @param fs [INOUT] face state data. This is required to initiate acceleration.
 *  @param acc_api [IN] a structure which contains acceleration APIs. If NULL, acceleration will be made disabled.
 *
 * Enable or disable acceleration on the ISP. If this function is not called, face engine will work without acceleration.
 */
LIBEXPORT
void
ia_face_set_acceleration(ia_face_state *fs, const ia_acceleration *acc_api);

/** @brief Try to detect faces from the input frame, assuming that the frame comes from the camera viewfinder.
 *
 *  @param fs [INOUT] face state data. Face detection result will be filled after the call.
 *  @param frame [IN] a frame where the engine will try to detect faces from.
 *
 *  This function tries to find faces from the input image. This function also uses information from the previous frames, so
 *  it is common practice to use this function to detect faces from the camera viewfinder.
 */
LIBEXPORT
void
ia_face_detect(ia_face_state *fs, const ia_frame *frame); /* uses face tracking and results of previous frame */

/** @brief Try to detect faces from the input frame, assuming that the frame comes from a still image.
 *
 *  @param fs [INOUT] face state data. Face detection result will be filled after the call.
 *  @param frame [IN] a frame where the engine will try to detect faces from.
 *
 *  This function tries to find faces from the input image, typically a captured still image.
 */
LIBEXPORT
void
ia_face_detect_in_image(ia_face_state *fs, const ia_frame *frame);

/** @brief Try to detect eye positions from the given input frame bounded by the face area.
 *
 *  @param fs [INOUT] face state data. Eye detection result will be filled.
 *  @param frame [IN] a frame where the engine will try to detect eyes from.
 *
 *  This function tries to find eye positions inside the face area of the input frame.
 *  Using face_area information in faces array in fs structure, this function will fill eye positions to left_eye and right_eye variable of corresponding structure.
 *  Therefore this function should be called only after face detection is performed.
 */
LIBEXPORT
void
ia_face_eye_detect(ia_face_state *fs, const ia_frame *frame);

/** @brief Try to detect mouth position from the face area of the input frame.
 *
 *  @param fs [INOUT] face state data. Mouth detection result will be filled.
 *  @param frame [IN] a frame where the engine will try to detect the mouth from.
 *
 *  This function tries to find the center position of the mouth inside the face area of the input frame.
 *  Using face_area information in faces array in fs structure, this function will fill the mouth field of corresponding structure.
 *  As in case of eye detection, this function also should be called after face detection.
 */
LIBEXPORT
void
ia_face_mouth_detect(ia_face_state *fs, const ia_frame *frame);

/** @brief Try to detect smile scores of each face in the input frame.
 *
 *  @param fs [INOUT] face state data. Smile detection result will be filled into faces array.
 *  @param frame [IN] a frame where the faces are in.
 *
 *  This function tries to find whether each face in the frame is smiling, and if yes, also tries to find how much.
 *  Face area and eye positions should be valid for calling this function to be meaningful.
 */
LIBEXPORT
void
ia_face_smile_detect(ia_face_state *fs, const ia_frame *frame);

/** @brief Try to detect blink states of each face in the input frame.
 *
 *  @param fs [INOUT] face state data. Blink detection result will be filled into faces array.
 *  @param frame [IN] a frame where the faces are in.
 *
 *  This function tries to find whether each eye is closed or open as in the range of 0 to 100.
 *  Face area and eye positions should be valid for calling this function to be meaningful.
 */
LIBEXPORT
void
ia_face_blink_detect(ia_face_state *fs, const ia_frame *frame);

/** @brief Try to recognize faces in the input frame.
 *
 *  @param fs [INOUT] face state date. Recognition result will be filled into faces array.
 *  @param frame [IN] a frame that contains faces.
 *
 *  For the face recognition to work properly, some preconditions should be met :
 *  - Face detection and eye detection results need to be valid.
 *  - Face recognition database (if exists) should be loaded by ia_face_set_feature_db() or ia_face_register_feature().
 *  - Using ISP acceleration is recommended to get the better/faster results.
 *
 *  This function is intended to be used on the camera preview frames, which means this function also uses temporal information
 *  to get better recognition result. For recognizing faces in still images, use ia_face_recognize_in_image().
 */
LIBEXPORT
void
ia_face_recognize(ia_face_state *fs, const ia_frame *frame);

/** @brief Try to recognize faces in the still image.
 *
 *  @param fs [INOUT] face state data. Recognition result will be filled into faces array.
 *  @param frame [IN] a frame that contains faces.
 *
 *  Works the same way as ia_face_recognize() except that this function should be used for still images.
 */
LIBEXPORT
void
ia_face_recognize_in_image(ia_face_state *fs, const ia_frame *frame);

/** @brief Try to recognize a single face in the still image.
 *
 *  @param fs [INOUT] face state data. Recognition result will be filled into faces array.
 *  @param frame [IN] a frame that contains a face.
 *  @param index_to_recognize [IN] index of the face to recognize. i.e. The face in fs->faces[index_to_recognize] will be processed.
 *  @param extracted_feature [OUT] face feature data generated inside the engine. Returned feature data ("data" field) is managed by the engine, so do not try to free it.
 *
 *  This function will try to recognize the face at the specified index and return the result. Recognition result will be filled into the fs, and
 *  the face feature data is also returned via the structure so as the caller can store it into external database.
 *
 *  As this function takes the face index to recognize, face detection must be done first. Calling eye detection is optional since it will be called internally
 *  if the result is not present in the fs structure.
 */
LIBEXPORT
int32_t
ia_face_recognize_single_in_image(ia_face_state *fs, const ia_frame *frame, int32_t index_to_recognize, ia_face_feature *extracted_feature);

/** @brief Unregister (or delete) a face from the face recognition database.
 *
 *  @param fs [IN] face state data.
 *  @param feature_id [IN] feature id of the face to unregister. This is returned value of ia_face_register_face().
 *
 *  Delete the face from the face recognition database. Unregistered faces will not be used for recognizing other faces any more.
 */
LIBEXPORT
ia_err
ia_face_feature_unregister(ia_face_state *fs, int32_t feature_id);

/** @brief Set/modify person_id of registered face.
 *
 *  @param fs [IN] face state data.
 *  @param feature_id [IN] feature id of the face to modify. This is returned value of ia_face_register_face().
 *  @param person_id [IN] new person id
 *
 *  This function modifies the person id of existing (already registered) face.
 */
LIBEXPORT
ia_err
ia_face_feature_set_person_id(ia_face_state *fs, int32_t feature_id, int32_t person_id);

/** @brief Unregister (or delete) a person and all faces associated with the person.
 *
 *  @param fs [IN] face state data.
 *  @param person_id [IN] person id to delete.
 *
 *  This function thoroughly deletes a person and all faces registered with the person id from the face recognition database.
 */
LIBEXPORT
ia_err
ia_face_unregister_person(ia_face_state *fs, int32_t person_id);

/** @brief Get the face recognition database in the memory as a byte stream.
 *
 *  @param fs [IN] face state data.
 *  @param db_size [OUT] size of database bytestream in bytes.
 *  @param db_data [OUT] pointer to database bytestream. Memory is allocated inside this function.
 *
 *  This function is typically used to store the face recognition database in the memory to the file system.
 */
LIBEXPORT
ia_err
ia_face_get_feature_db(ia_face_state *fs, int32_t *db_size, const uint8_t **db_data);

/** @brief Set the given bytestream as the face recognition database.
 *
 *  @param fs [IN] face state data.
 *  @param db_size [IN] size of the bytestream in bytes.
 *  @param db_data [IN] database bytestream.
 *
 *  This function copies a bytestream to engine internal memory as the face recognition database.
 *  ia_face_register_feature() also can be used to copy one database entry into the memory.
 */
LIBEXPORT
ia_err
ia_face_set_feature_db(ia_face_state *fs, int32_t db_size, const uint8_t *db_data);

/** @brief Register (store) the given face feature to the database in the memory.
 *  @deprecated
 *
 *  @param fs [IN] face state data.
 *  @param new_feature [IN] feature data
 *  @param new_person_id [IN] person id associated with this face feature
 *  @param new_feature_id [IN] feature id associated with this face feature
 *  @param time_stamp [IN] timestamp of this face feature
 *  @param condition [IN] condition value
 *  @param checksum [IN] checksum value
 *  @param version [IN] version value
 *  @return feature id which is the same new_feature_id in case of success. Will be a negative value on error.
 *
 *  This function copies one feature data to engine internal memory as the face recognition database.
 *  For bulk data, ia_face_set_feature_db() can be used.
 *  Users don't have to concern about meaning of parameters, but only need to pass each parameter as received
 *   from database manager (e.g. in case of Android, SQLite).
 *
 *  This function is deprecated. Please use ia_face_register_face_feature() instead.
 */
LIBEXPORT
int32_t
ia_face_register_feature(ia_face_state* fs, const uint8_t *new_feature, int32_t new_person_id,
                         int32_t new_feature_id, int32_t time_stamp, int32_t condition, int32_t checksum, int32_t version);

/** @brief Register (store) the given face feature to the database in the memory.
 *
 *  @param fs [IN] face state data.
 *  @param feature_to_register [IN] face feature data to store.
 *
 *  This function copies one feature data to internal face recognition database.
 *  Basically there are two use cases of this function :
 *   - At the engine initialization stage, use this function to copy face feature data from external database to internal database.
 *   - When a new photo is imported, use this function to register a newly detected/recognized face into database.
 */
LIBEXPORT
int32_t
ia_face_register_face_feature(ia_face_state *fs, const ia_face_feature *feature_to_register);

/** @brief Detect/track headpose information from a face.
 *  @deprecated
 *
 *  @param fs [IN] Face engine state data.
 *  @param headpose [OUT] Headpose information structure where output will be stored. Caller must manage the memory for this struct.
 *  @param frame [IN] Image frame with faces.
 *  @param tracking_id [IN] Tracking Id of the face as returned by ia_face_detect().
 *
 *  This function detects/tracks the head pose information of a face inside the input frame. Please refer to ia_face_headpose structure for the available information.
 *  Before calling this function, face detection information should be available and the tracking id of the face to track headpose needs to be passed as an argument.
 *  However please note that once a face is detected by ia_face_detect(), the face information (ia_face struct) will not be used for headpose tracking, meaning that
 *  users don't need to call another ia_face_detect() for headpose tracking.
 *  If returned headpose->processing_state is zero, it means headpose tracking has failed and user needs to start over by calling face detection first.
 */
LIBEXPORT
ia_err
ia_face_track_headpose(ia_face_state *fs, ia_face_headpose *headpose, const ia_frame *frame, int32_t tracking_id);

/** @brief Apply face beautification filter on the image.
 *
 *  @param fs [IN] Face engine state data.
 *  @param output [OUT] Output frame where processed image will be stored. Caller must manage the memory for this.
 *  @param input [IN] Input frame to process.
 *  @param strength [IN] Specify the strength of the filter. Values between 0 (bypass) and 9 (strongest) can be specified.
 *
 *  This function applies face beautification filter on the input image. Depending on the implementation, this function may use face detection information
 *  in the fs structure, in which case face detection may be called internally if it hasn't been called.
 *  Output frame must be allocated and freed by the caller.
 */
LIBEXPORT
ia_err
ia_face_beautify(ia_face_state *fs, ia_frame *output, const ia_frame *input, int32_t strength);

/** @brief Find skin type likelihood from the faces already found.
 *
 *  @param fs [IN] Face engine state data.
 *  @param frame [IN] a frame where the faces are in.
 *
 *  This function attempts to detect the skin type of the faces which are already found and stored in the handle,
 *  results the dark-skin likelihood back to the member 'skin_type_dark_likelihood' and 'skin_type_validity' in each ia_faces.
 */
LIBEXPORT
void
ia_face_detect_skin(ia_face_state *fs, const ia_frame *frame);

#ifdef __cplusplus
}
#endif

#endif /* _IA_FACE_H_ */
