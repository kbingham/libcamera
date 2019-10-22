/*
 * bcm2835_isp.h
 */

#include <stdint.h>

#ifndef _BCM2835_ISP_H_
#define _BCM2835_ISP_H_

/*
 * This file should come from the UAPI exported by the kernel, and ideally
 * should live in the kernel, but will be duplicated to libcamera to support
 * building against older kernel versions.
 */

/*
 * Stats buffer types are not yet defined, so just some dummy parameters for
 * now. These should be provided by the RPI ISP UAPI Header.
 */
struct rpi_isp_params_cfg {
	bool aeEnable;
};

struct rpi_stat_buffer {
	uint32_t exposure;
};

#endif /* _BCM2835_ISP_H_ */
