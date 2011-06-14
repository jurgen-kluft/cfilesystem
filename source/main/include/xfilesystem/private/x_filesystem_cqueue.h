#ifndef __X_FILESYSTEM_CONCURRENT_QUEUE_H__
#define __X_FILESYSTEM_CONCURRENT_QUEUE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xfilesystem\private\x_filesystem_constants.h"

namespace xcore
{
	namespace xfilesystem
	{
		/// A concurrent sliding window class for the user to detect items
		/// not being in the queue or to detect if they are still in the queue
		class cswindow
		{
		public:
			bool					push(xasync_id& outId)					{ outId = mTail; ++mTail; }
			bool					pop(xasync_id& outId)					{ outId = mHead; ++mHead; }

			bool					inside(xasync_id id) const				{ return id>=mHead && id<mTail; }
			bool					outside(xasync_id id) const				{ return id<mHead;}

		private:
			// These integers need to be atomic
			xasync_id				mTail;			/// Items are added to the tail (grow)
			xasync_id				mHead;			/// Items are removed from the head (shrink)
		};

		/// A concurrent queue, multiple producers, multiple consumers
		template<typename TElement>
		class cqueue
		{
		public:
									cqueue()
										: mTail(0)
										, mHead(0)
										, mCapacity(0)
										, mArray(NULL)			{ }

									cqueue(s32 _capacity, TElement volatile* _array) 
										: mTail(0)
										, mHead(0)
										, mCapacity(_capacity)
										, mArray(_array)		{ }

			XFILESYSTEM_OBJECT_NEW_DELETE()

			xcore::xbool			push(TElement item_, u32& outIndex);
			xcore::xbool			pop(TElement& item_);
			xcore::xbool			peek(TElement& item_);

			bool					inside(s32 index) const					{ return index>=mHead && index<mTail; }
			bool					outside(s32 index) const				{ return index<mHead;}

			xcore::xbool			empty() const;
			xcore::xbool			full() const;
			xcore::u32				size() const;

			TElement volatile*		getArray() const;

		private:
			volatile xcore::s32		mTail;											// input index
			xcore::xbyte			padding1[CACHE_LINE_SIZE-sizeof(xcore::u32)];
			volatile xcore::s32		mHead;											// output index
			xcore::xbyte			padding2[CACHE_LINE_SIZE-sizeof(xcore::u32)];
			volatile xcore::s32		mCapacity;										// capacity
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
		xcore::xbool cqueue<TElement>::push(TElement item_, u32& outIndex)
		{
			xcore::s32 nextTail = increment(mTail);
			if(nextTail != mHead)
			{
				mArray[mTail % mCapacity] = item_;
				outIndex = mTail;
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
		xcore::xbool cqueue<TElement>::pop(TElement& item_)
		{
			if(mHead == mTail)
				return false;  // empty queue

			item_ = mArray[mHead % mCapacity];
			mHead = increment(mHead);
			return true;
		}
		template<typename TElement>
		xcore::xbool cqueue<TElement>::peek(TElement& item_)
		{
			if(mHead == mTail)
				return false;  // empty queue

			item_ = mArray[mHead % mCapacity];
			return true;
		}

		/// Useful for testing and Consumer check of status
		/// Remember that the 'empty' status can change quickly
		/// as the Producer adds more items.
		///
		/// \return true if circular buffer is empty */
		template<typename TElement>
		xcore::xbool cqueue<TElement>::empty() const
		{
			return (mHead == mTail);
		}

		/// Useful for testing and Producer check of status
		/// Remember that the 'full' status can change quickly
		/// as the Consumer catches up.
		///
		/// \return true if circular buffer is full.  */
		template<typename TElement>
		xcore::xbool cqueue<TElement>::full() const
		{
			xcore::s32 diff = mTail - mHead;
			if (diff < 0) 
				diff += mCapacity;
			return (xcore::u32)diff == mCapacity;
		}

		/// Returns the number of items in the queue
		///
		/// \return size.  */
		template<typename TElement>
		xcore::u32 cqueue<TElement>::size() const
		{
			xcore::s32 diff = mTail - mHead;
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
		xcore::u32 cqueue<TElement>::increment(xcore::u32 idx_) const
		{
			// increment or wrap
			// =================
			//    index++;
			//    if(index == mArray.lenght) -> index = 0;
			//
			//or as written below:   
			//    index = (index+1) % mArray.length
			idx_ = (idx_+1);
			return idx_;
		}

		template<typename TElement>
		TElement volatile* cqueue<TElement>::getArray() const
		{
			return mArray;
		}
	}
}

#endif ///< __X_FILESYSTEM_CONCURRENT_QUEUE_H__
