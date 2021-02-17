/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2014-2018 Intel Corporation
 *
 * This implementation is highly derived from ChromeOS:
 *   platform2/camera/hal/intel/ipu3/common/SharedItemPool.cpp
 */

#include <ia_imaging/ia_aiq_types.h>

#include "libcamera/internal/log.h"

#include "shared_item_pool.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(SharedItemPool)

template<class ItemType>
SharedItemPool<ItemType>::SharedItemPool(const char *name)
	: allocated_(nullptr), capacity_(0), deleter_(this), poolName_(name),
	  resetter_(nullptr)
{
}

template<class ItemType>
SharedItemPool<ItemType>::~SharedItemPool()
{
	deInit();
}

template<class ItemType>
int SharedItemPool<ItemType>::init(int32_t capacity, void (*resetter)(ItemType *))
{
	if (capacity_ != 0) {
		LOG(SharedItemPool, Error) << "Pool initialized already";
		return -ENOSYS;
	}
	std::lock_guard<std::mutex> l(mutex_);
	resetter_ = resetter;
	capacity_ = capacity;
	available_.reserve(capacity);
	allocated_ = new ItemType[capacity];

	for (int32_t i = 0; i < capacity; i++)
		available_.push_back(&allocated_[i]);

	LOG(SharedItemPool, Debug) << "Shared pool " << poolName_ << "init with " << capacity << " items";

	return 0;
}

template<class ItemType>
bool SharedItemPool<ItemType>::isFull()
{
	std::lock_guard<std::mutex> l(mutex_);
	bool ret = (available_.size() == capacity_);
	return ret;
}

template<class ItemType>
int SharedItemPool<ItemType>::deInit()
{
	std::lock_guard<std::mutex> l(mutex_);
	if (capacity_ == 0) {
		LOG(SharedItemPool, Debug) << "Shared pool " << poolName_
					   << " isn't initialized or already de-initialized";
		return 0;
	}
	if (available_.size() != capacity_) {
		LOG(SharedItemPool, Error) << "Not all items are returned "
					   << "when destroying pool " << poolName_
					   << "(" << available_.size() << "/" << capacity_ << ")";
	}

	delete[] allocated_;
	allocated_ = nullptr;
	available_.clear();
	capacity_ = 0;
	LOG(SharedItemPool, Debug) << "Shared pool " << poolName_
				   << " deinit done";

	return 0;
}

template<class ItemType>
int SharedItemPool<ItemType>::acquireItem(std::shared_ptr<ItemType> &item)
{
	item.reset();
	std::lock_guard<std::mutex> l(mutex_);
	if (available_.empty()) {
		LOG(SharedItemPool, Error) << "Shared pool " << poolName_
					   << "is empty";
		return -ENOSYS;
	}

	std::shared_ptr<ItemType> sh(available_[0], deleter_);
	available_.erase(available_.begin());
	item = sh;
	LOG(SharedItemPool, Debug) << "Shared pool " << poolName_
				   << "acquire items " << sh.get();
	return 0;
}

template<class ItemType>
size_t SharedItemPool<ItemType>::availableItems()
{
	std::lock_guard<std::mutex> l(mutex_);
	size_t ret = available_.size();
	return ret;
}

template<class ItemType>
int SharedItemPool<ItemType>::_releaseItem(ItemType *item)
{
	std::lock_guard<std::mutex> l(mutex_);
	if (resetter_)
		resetter_(item);

	LOG(SharedItemPool, Debug) << "Shared pool " << poolName_
				   << "returning item " << item;

	available_.push_back(item);
	return 0;
}

template class SharedItemPool<ia_aiq_rgbs_grid>;
template class SharedItemPool<ia_aiq_af_grid>;
} /* namespace libcamera */
