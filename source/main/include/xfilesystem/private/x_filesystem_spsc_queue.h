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

namespace xcore
{
	namespace xfilesystem
	{
		/// Circular Fifo (a.k.a. Circular Buffer) 
		/// Thread safe for one reader, and one writer
		template<typename TElement, xcore::u32 TSize>
		class spsc_cqueue
		{
		public:
			enum
			{
				capacity = TSize
			};
									spsc_cqueue() : mTail(0), mHead(0)						{ }

			xcore::xbool			push(TElement item_);
			xcore::xbool			pop(TElement& item_);

			xcore::xbool			empty() const;
			xcore::xbool			full() const;
			xcore::u32				size() const;

			void*					operator new (size_t size, void *p)					{ return p; }
			void					operator delete(void* mem, void* )					{ }	

		private:
			volatile xcore::u32		mTail;											// input index
			xcore::xbyte			padding1[CACHE_LINE_SIZE-sizeof(xcore::u32)];
			volatile xcore::u32		mHead;											// output index
			xcore::xbyte			padding2[CACHE_LINE_SIZE-sizeof(xcore::u32)];
			volatile TElement		mArray[capacity];

			xcore::u32				increment(xcore::u32 idx_) const;
		};

		///< Producer only: Adds item to the circular queue. 
		/// If queue is full at 'push' operation no update/overwrite
		/// will happen, it is up to the caller to handle this case
		///
		/// \param item_ copy by reference the input item
		/// \return whether operation was successful or not
		template<typename TElement, xcore::u32 TSize>
		xcore::xbool spsc_cqueue<TElement, TSize>::push(TElement item_)
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
		template<typename TElement, xcore::u32 TSize>
		xcore::xbool spsc_cqueue<TElement, TSize>::pop(TElement& item_)
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
		template<typename TElement, xcore::u32 TSize>
		xcore::xbool spsc_cqueue<TElement, TSize>::empty() const
		{
			return (mHead == mTail);
		}

		/// Useful for testing and Producer check of status
		/// Remember that the 'full' status can change quickly
		/// as the Consumer catches up.
		///
		/// \return true if circular buffer is full.  */
		template<typename TElement, xcore::u32 TSize>
		xcore::xbool spsc_cqueue<TElement, TSize>::full() const
		{
			xcore::s32 tailCheck = (mTail+1) % capacity;
			return (tailCheck == mHead);
		}

		/// Returns the number of items in the queue
		///
		/// \return size.  */
		template<typename TElement, xcore::u32 TSize>
		xcore::u32 spsc_cqueue<TElement, TSize>::size() const
		{
			xcore::s32 diff = mHead - mTail;
			if (diff < 0) 
				diff += TSize;
			return (xcore::u32)diff;
		}

		/// Increment helper function for index of the circular queue
		/// index is incremented or wrapped
		///
		///  \param idx_ the index to the incremented/wrapped
		///  \return new value for the index */
		template<typename TElement, xcore::u32 TSize>
		xcore::u32 spsc_cqueue<TElement, TSize>::increment(xcore::u32 idx_) const
		{
			// increment or wrap
			// =================
			//    index++;
			//    if(index == mArray.lenght) -> index = 0;
			//
			//or as written below:   
			//    index = (index+1) % mArray.length
			idx_ = (idx_+1) % capacity;
			return idx_;
		}

	}
}

#endif ///< __X_FILESYSTEM_SPSC_QUEUE_H__