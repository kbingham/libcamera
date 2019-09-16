/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * ipa_proxy_linux_protocol.h - IPA Proxy Linux protocol
 */
#ifndef __IPA_PROXY_LINUX_PROTOCOL_H__
#define __IPA_PROXY_LINUX_PROTOCOL_H__

namespace libcamera {

namespace IPAProxyLinux {

enum MessageType {
};

struct Message {
	enum MessageType type;
};

} /* namespace IPAProxyLinux */

} /* namespace libcamera */

#endif /* __IPA_PROXY_LINUX_PROTOCOL_H__ */
