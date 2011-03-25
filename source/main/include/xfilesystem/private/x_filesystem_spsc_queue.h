#ifndef __X_FILESYSTEM_SPSC_QUEUE_H__
#define __X_FILESYSTEM_SPSC_QUEUE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xfilesystem\private\x_filesystem_common.h"

namespace xcore
{
	namespace xfilesystem
	{
		/// Circular Fifo (a.k.a. Circular Buffer) 
		/// Thread safe for one reader, and one writer
		template<typename TElement>
		class spsc_cqueue
		{
		public:
									spsc_cqueue()
										: mTail(0)
										, mHead(0)
										, mCapacity(0)
										, mArray(NULL)			{ }

									spsc_cqueue(s32 _capacity, TElement volatile* _array) 
										: mTail(0)
										, mHead(0)
										, mCapacity(_capacity)
										, mArray(_array)		{ }

			XFILESYSTEM_OBJECT_NEW_DELETE()

			xcore::xbool			push(TElement item_);
			xcore::xbool			pop(TElement& item_);

			xcore::xbool			empty() const;
			xcore::xbool			full() const;
			xcore::u32				size() const;

			TElement volatile*		getArray() const;

		private:
			volatile xcore::u32		mTail;											// input index
			xcore::xbyte			padding1[CACHE_LINE_SIZE-sizeof(xcore::u32)];
			volatile xcore::u32		mHead;											// output index
			xcore::xbyte			padding2[CACHE_LINE_SIZE-sizeof(xcore::u32)];
			volatile xcore::u32		mCapacity;										// capacity
			xcore::xbyte			padding3[CACHE_LINE_SIZE-sizeof(xcore::u32)];
			TElement volatile		*mArray;

			xcore::u32				increment(xcore::u32 idx_) const;
		};

		///< Producer only: Adds item to the circular queue. 
		/// If queue is full at 'push' operation no update/overwrite
		/// will happen, it is up to the caller to handle this case
		///
		/// \param item_ copy by reference the input item
		/// \return whether operation was successful or not
		template<typename TElement>
		xcore::xbool spsc_cqueue<TElement>::push(TElement item_)
		{
			xcore::s32 nextTail = increment(mTail);
			if(nextTail != mHead)
			{
				mArray[mTail] = item_;
				mTail = nextTail;
				return true;
			}

			// queue was full
			return false;
		}

		/// Consumer only: Removes and returns item from the queue
		/// If queue is empty at 'pop' operation no retrieve will happen
		/// It is up to the caller to handle this case
		///
		/// \param item_ return by reference the wanted item
		/// \return whether operation was successful or not */
		template<typename TElement>
		xcore::xbool spsc_cqueue<TElement>::pop(TElement& item_)
		{
			if(mHead == mTail)
				return false;  // empty queue

			item_ = mArray[mHead];
			mHead = increment(mHead);
			return true;
		}

		/// Useful for testing and Consumer check of status
		/// Remember that the 'empty' status can change quickly
		/// as the Producer adds more items.
		///
		/// \return true if circular buffer is empty */
		template<typename TElement>
		xcore::xbool spsc_cqueue<TElement>::empty() const
		{
			return (mHead == mTail);
		}

		/// Useful for testing and Producer check of status
		/// Remember that the 'full' status can change quickly
		/// as the Consumer catches up.
		///
		/// \return true if circular buffer is full.  */
		template<typename TElement>
		xcore::xbool spsc_cqueue<TElement>::full() const
		{
			xcore::s32 tailCheck = (mTail+1) % mCapacity;
			return (tailCheck == mHead);
		}

		/// Returns the number of items in the queue
		///
		/// \return size.  */
		template<typename TElement>
		xcore::u32 spsc_cqueue<TElement>::size() const
		{
			xcore::s32 diff = mHead - mTail;
			if (diff < 0) 
				diff += mCapacity;
			return (xcore::u32)diff;
		}

		/// Increment helper function for index of the circular queue
		/// index is incremented or wrapped
		///
		///  \param idx_ the index to the incremented/wrapped
		///  \return new value for the index */
		template<typename TElement>
		xcore::u32 spsc_cqueue<TElement>::increment(xcore::u32 idx_) const
		{
			// increment or wrap
			// =================
			//    index++;
			//    if(index == mArray.lenght) -> index = 0;
			//
			//or as written below:   
			//    index = (index+1) % mArray.length
			idx_ = (idx_+1) % mCapacity;
			return idx_;
		}

		template<typename TElement>
		TElement volatile* spsc_cqueue<TElement>::getArray() const
		{
			return mArray;
		}
	}
}

#endif ///< __X_FILESYSTEM_SPSC_QUEUE_H__