/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * ipa_proxy_linux_worker.cpp - Default Image Processing Algorithm proxy worker for Linux
 */

#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include <ipa/ipa_interface.h>
#include <libcamera/event_dispatcher.h>
#include <libcamera/logging.h>
#include <libcamera/object.h>

#include "ipa_module.h"
#include "ipc_unixsocket.h"
#include "log.h"
#include "thread.h"
#include "utils.h"

#include "event_loop.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPAProxyLinuxWorker)

namespace IPAProxyLinux {

class Worker : public Object
{
public:
	Worker(const char *module, int socket);
	~Worker();

	bool isValid() { return context_ != nullptr; }

	int exec();

private:
	void readyRead(IPCUnixSocket *ipc);

	EventLoop loop_;
	IPCUnixSocket socket_;
	std::unique_ptr<IPAModule> module_;
	struct ipa_context *context_;
};

Worker::Worker(const char *module, int socket)
	: context_(nullptr)
{
	LOG(IPAProxyLinuxWorker, Debug)
		<< "Starting worker for IPA module '" << module
		<< "' with IPC socket " << socket;

	module_ = utils::make_unique<IPAModule>(module);
	if (!module_->isValid() || !module_->load()) {
		LOG(IPAProxyLinuxWorker, Error)
			<< "IPAModule " << module << " should be valid but isn't";
		return;
	}

	socket_.readyRead.connect(this, &Worker::readyRead);
	if (socket_.bind(socket) < 0) {
		LOG(IPAProxyLinuxWorker, Error) << "IPC socket binding failed";
		return;
	}

	context_ = module_->createContext();
	if (!context_) {
		LOG(IPAProxyLinuxWorker, Error) << "Failed to create IPA context";
		return;
	}

	LOG(IPAProxyLinuxWorker, Debug) << "Proxy worker successfully started";
}

Worker::~Worker()
{
	if (context_)
		context_->ops->destroy(context_);
}

int Worker::exec()
{
	return loop_.exec();
}

void Worker::readyRead(IPCUnixSocket *ipc)
{
	IPCUnixSocket::Payload payload;
	int ret;

	ret = ipc->receive(&payload);
	if (ret) {
		LOG(IPAProxyLinuxWorker, Error)
			<< "Receive message failed: " << ret;
		return;
	}

	LOG(IPAProxyLinuxWorker, Debug) << "Received a message!";
}

} /* namespace IPAProxyLinux */

} /* namespace libcamera */

using namespace libcamera;

int main(int argc, char **argv)
{
	/* Uncomment this for debugging. */
#if 0
	std::string logPath = "/tmp/libcamera.worker." +
			      std::to_string(getpid()) + ".log";
	logSetFile(logPath.c_str());
#endif

	if (argc < 3) {
		LOG(IPAProxyLinuxWorker, Debug)
			<< "Tried to start worker with no args";
		return EXIT_FAILURE;
	}

	int fd = std::stoi(argv[2]);

	IPAProxyLinux::Worker worker(argv[1], fd);
	if (!worker.isValid())
		return EXIT_FAILURE;

	return worker.exec();
}
