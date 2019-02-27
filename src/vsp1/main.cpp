/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * main.cpp - vsp1 - The vsp1 swiss army knife
 */

#include <iomanip>
#include <iostream>
#include <map>
#include <signal.h>
#include <string.h>

#include "options.h"
#include "vsp1.h"

using namespace std;

OptionsParser::Options options;

enum {
	OptDevice = 'd',
	OptCapture = 'C',
	OptFile = 'F',
	OptHelp = 'h',
	OptList = 'l',
};

void signalHandler(int signal)
{
	std::cout << "Exiting" << std::endl;
	exit(1);
}

static int parseOptions(int argc, char *argv[])
{
	KeyValueParser formatKeyValue;
	formatKeyValue.addOption("width", OptionInteger, "Width in pixels",
				 ArgumentRequired);
	formatKeyValue.addOption("height", OptionInteger, "Height in pixels",
				 ArgumentRequired);
	formatKeyValue.addOption("pixelformat", OptionInteger, "Pixel format",
				 ArgumentRequired);

	OptionsParser parser;
	parser.addOption(OptDevice, OptionString,
			 "Specify which device to operate on", "device",
			 ArgumentRequired, "device");
	parser.addOption(OptCapture, OptionNone,
			 "Capture until interrupted by user", "capture");
	parser.addOption(OptFile, OptionString,
			 "Write captured frames to disk\n"
			 "The first '#' character in the file name is expanded to the frame sequence number.\n"
			 "The default file name is 'frame-#.bin'.",
			 "file", ArgumentOptional, "filename");

	parser.addOption(OptHelp, OptionNone, "Display this help message",
			 "help");
	parser.addOption(OptList, OptionNone, "List all cameras", "list");

	options = parser.parse(argc, argv);
	if (!options.valid() || options.isSet(OptHelp)) {
		parser.usage();
		return !options.valid() ? -EINVAL : -EINTR;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret;

	ret = parseOptions(argc, argv);
	if (ret < 0)
		return ret == -EINTR ? 0 : EXIT_FAILURE;

	struct sigaction sa = {};
	sa.sa_handler = &signalHandler;
	sigaction(SIGINT, &sa, nullptr);

	VSP1 vsp1;

	ret = vsp1.init();
	if (ret) {
		cerr << "Failed to initiate VSP1" << endl;
		return EXIT_FAILURE;
	}

	if (options.isSet(OptList)) {
		std::cout << "Available VSP1:" << std::endl;
		vsp1.printMediaGraph();

	}

	if (options.isSet(OptDevice)) {
	}

	if (options.isSet(OptCapture)) {
	}

	//while(1);

	return ret;
}
