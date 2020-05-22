/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * main.h - Cam application
 */
#ifndef __CAM_MAIN_H__
#define __CAM_MAIN_H__

enum {
	OptCamera = 'c',
	OptCapture = 'C',
	OptDisplay = 'D',
	OptFile = 'F',
	OptHelp = 'h',
	OptInfo = 'I',
	OptList = 'l',
	OptListProperties = 'p',
	OptStream = 's',

	/* Following enums are to be above 256 */
	OptListLongOnlyArguments = 256,
	OptListControls,
	OptListFormats,

};

#endif /* __CAM_MAIN_H__ */
