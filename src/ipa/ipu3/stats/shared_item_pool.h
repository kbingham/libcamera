/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2014-2018 Intel Corporation
 *
 * This implementation is highly derived from ChromeOS:
 *   platform2/camera/hal/intel/ipu3/common/SharedItemPool.h
 */

#ifndef SHARED_ITEM_POOL_H
#define SHARED_ITEM_POOL_H

#include <memory>
#include <mutex>
#include <pthread.h>
#include <vector>

/**
 * \class SharedItemPool
 *
 * Pool of ref counted items. This class creates a pool of items and manages
 * the acquisition of them. When all references to this item have disappeared
 * The item is returned to the pool.
 *
 * This class is thread safe, i.e. it can be called from  multiple threads.
 * When the element is recycled to the pool it can be reset via a client
 * provided method.
 *
 */

namespace libcamera {

template<class ItemType>
class SharedItemPool
{
public:
	SharedItemPool(const char *name = "Unnamed");
	~SharedItemPool();

	/**
	 * Initializes the capacity of the pool. It allocates the objects.
	 * optionally it will take function to reset the item before recycling
	 * it to the pool.
	 * This method is thread safe.
	 *
	 * \param capacity[IN]: Number of items the pool will hold
	 * \param resetter[IN]: Function to reset the item before recycling to the
	 *                      pool.
	 * \return -ENOSYS if trying to initialize twice
	 * \return 0 If everything went ok.
	 */
	int init(int32_t capacity, void (*resetter)(ItemType *) = nullptr);

	bool isFull();

	/**
	 * Free the resources of the pool
	 *
	 * \return 0 on success
	 */
	int deInit();

	/**
	 * Acquire an item from the pool.
	 * This method is thread safe. Access to the internal acquire/release
	 * methods are protected.
	 * BUT the thread-safety for the utilization of the item after it has been
	 * acquired is the user's responsibility.
	 * Be careful not to provide the same item to multiple threads that write
	 * into it.
	 *
	 * \param item[OUT] shared pointer to an item.
	 * \return 0 on success
	 */
	int acquireItem(std::shared_ptr<ItemType> &item);

	/**
	 * Returns the number of currently available items
	 * It there would be issues acquiring the lock the method returns 0
	 * available items.
	 *
	 * \return item count
	 */
	size_t availableItems();

private:
	int _releaseItem(ItemType *item);

	class ItemDeleter
	{
	public:
		ItemDeleter(SharedItemPool *pool)
			: mPool(pool) {}
		void operator()(ItemType *item) const
		{
			mPool->_releaseItem(item);
		}

	private:
		SharedItemPool *mPool;
	};

	std::vector<ItemType *> available_; /* SharedItemPool doesn't have ownership */
	ItemType *allocated_;
	size_t capacity_;
	ItemDeleter deleter_;
	std::mutex mutex_; /* protects available_, allocated_, capacity_ */
	const char *poolName_;
	void (*resetter_)(ItemType *);
};

} /* namespace libcamera */

#endif /* SHARED_ITEM_POOL_H */

