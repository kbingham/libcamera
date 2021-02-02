/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Wrapper to be able to include KBL_AIC implementation without reducing our
 * compiler warnings and error levels on our own code.
 */

#ifndef IPA_IPU3_AIC_KBL_AIC_H
#define IPA_IPU3_AIC_KBL_AIC_H

/*
 * Bring in AIC headers with compiler warnings disabled on those includes.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wextra-semi"

#include <ia_imaging/KBL_AIC.h>

#pragma GCC diagnostic pop

#endif /* IPA_IPU3_AIC_H */
