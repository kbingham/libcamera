/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * vsp1.cpp - vsp1 - The vsp1 pipeline handler
 */

#include <iomanip>
#include <iostream>
#include <map>
#include <signal.h>
#include <string.h>

#include "vsp1.h"

using namespace std;
using namespace libcamera;

static void printNode(const MediaPad *pad, ostream &os)
{
	const MediaEntity *entity = pad->entity();

	os << "\"" << entity->name() << "\"["
	   << pad->index() << "]";
}

static void printLinkFlags(const MediaLink *link, ostream &os)
{
	unsigned int flags = link->flags();

	os << " [";
	if (flags) {
		os << (flags & MEDIA_LNK_FL_ENABLED ? "ENABLED," : "")
		   << (flags & MEDIA_LNK_FL_IMMUTABLE ? "IMMUTABLE" : "");
	}
	os  << "]\n";
}

/*
 * For each entity in the media graph, printout links directed to its sinks
 * and source pads.
 */
static void printMediaGraph(const MediaDevice &media, ostream &os)
{
	os << "\n" << media.driver() << " - " << media.deviceNode() << "\n\n";

	for (auto const &entity : media.entities()) {
		os << "\"" << entity->name() << "\"\n";

		for (auto const &sink : entity->pads()) {
			if (!(sink->flags() & MEDIA_PAD_FL_SINK))
				continue;

			os << "  [" << sink->index() << "]" << ": Sink\n";
			for (auto const &link : sink->links()) {
				os << "\t";
				printNode(sink, os);
				os << " <- ";
				printNode(link->source(), os);
				printLinkFlags(link, os);
			}
			os << "\n";
		}

		for (auto const &source : entity->pads()) {
			if (!(source->flags() & MEDIA_PAD_FL_SOURCE))
				continue;

			os << "  [" << source->index() << "]" << ": Source\n";
			for (auto const &link : source->links()) {
				os << "\t";
				printNode(source, os);
				os << " -> ";
				printNode(link->sink(), os);
				printLinkFlags(link, os);
			}
			os << "\n";
		}
	}

	os.flush();
}

static void followEntityPipeline(const MediaEntity *entity, ostream &os, unsigned int indent = 0)
{
	os << "\"" << entity->name() << "\"";
	indent += 4;

	for (auto const &source : entity->pads()) {
		if (!(source->flags() & MEDIA_PAD_FL_SOURCE))
			continue;

		/* Only follow enabled links */
		for (auto const &link : source->links()) {
			if (!(link->flags() & MEDIA_LNK_FL_ENABLED))
				continue;

			os << "[" << source->index() << "]" << std::endl;
			os << std::string( indent, ' ' ) << "-> [" << link->sink()->index() << "]";

			followEntityPipeline(link->sink()->entity(), os, indent);
		}
		os << "\n";
	}

	os.flush();
}

VSP1::~VSP1()
{
	if (media_)
		media_->release();

	delete rpfVideo_;
	delete wpfVideo_;
}

static int enableLink(MediaEntity *source, MediaEntity *sink)
{
	for (auto const &pad : source->pads()) {
		if (!(pad->flags() & MEDIA_PAD_FL_SOURCE))
			continue;

		for (MediaLink *link : pad->links()) {
			if (link->sink()->entity() == sink) {
				link->setEnabled(true);
				return 0;
			}
		}
	}

	cerr << "Failed to enable link" << endl;
	return -EINVAL;
}

int VSP1::init()
{
	enumerator_ = DeviceEnumerator::create();
	if (!enumerator_) {
		cerr << "Failed to create device enumerator" << endl;
		return -ENODEV;
	}

	if (enumerator_->enumerate()) {
		cerr << "Failed to enumerate media devices" << endl;
		return -ENODEV;
	}

	DeviceMatch dm("vsp1");
	dm.add("fe9b0000.vsp rpf.0 input");
	dm.add("fe9b0000.vsp wpf.0 output");

	media_ = enumerator_->search(dm);
	if (!media_) {
		cerr << "Failed to find a VSP device" << endl;
		return -ENODEV;
	}

	media_->acquire();

	if (media_->valid())
		cerr << "VSP1 media device is valid" << endl;

	rpf_ = media_->getEntityByName("fe9b0000.vsp rpf.0");
	uds_ = media_->getEntityByName("fe9b0000.vsp uds.0");
	sru_ = media_->getEntityByName("fe9b0000.vsp sru");
	wpf_ = media_->getEntityByName("fe9b0000.vsp wpf.0");

	if (!rpf_ || !uds_ || !sru_ || !wpf_) {
		cerr << "Failed to get entities" << endl;
		cerr << " RPF: " << rpf_
		     << " UDS: " << uds_
		     << " SRU: " << sru_
		     << " WPF: " << wpf_
		     << endl;
		return -ENODEV;
	}

	if (media_->disableLinks()) {
		cerr << "Failed to reset links" << endl;
		return -EINVAL;
	}

	/* Default to RPF->WPF linkage */
	if (enableLink(rpf_, wpf_))
		return -ENOLINK;

	followEntityPipeline(rpf_, cerr);

	rpfVideo_ = V4L2VideoDevice::fromEntityName(media_.get(), "fe9b0000.vsp rpf.0 input");
	wpfVideo_ = V4L2VideoDevice::fromEntityName(media_.get(), "fe9b0000.vsp wpf.0 output");

	rpfVideo_->open();
	wpfVideo_->open();

	cerr << "RPF: deviceName: " << rpfVideo_->deviceName()
	     << " driverName: " << rpfVideo_->driverName()
	     << " deviceNode: " << rpfVideo_->deviceNode()
	     << endl;

	cerr << "WPF: deviceName: " << wpfVideo_->deviceName()
	     << " driverName: " << wpfVideo_->driverName()
	     << " deviceNode: " << wpfVideo_->deviceNode()
	     << endl;

	return 0;
}

void VSP1::printMediaGraph()
{
	::printMediaGraph(*media_, cerr);
}
