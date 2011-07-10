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
#include "xbase\x_integer.h"
#include "xatomic\x_atomic.h"
#include "xatomic\x_queue.h"
#include "xfilesystem\private\x_filesystem_constants.h"
#include "xfilesystem\private\x_filesystem_constants.h"

namespace xcore
{
	namespace xfilesystem
	{
		/// A concurrent queue, multiple producers, multiple consumers
		template<typename TElement>
		class cqueue
		{
		public:
									cqueue()								{ }

			XFILESYSTEM_OBJECT_NEW_DELETE()

			bool					init(x_iallocator* _allocator, u32 size)
			{
				mFifo = (atomic::fifo::link*)_allocator->allocate((size+1) * sizeof(atomic::fifo::link), 8);
				mLifo = (atomic::lifo::link*)_allocator->allocate((size+1) * sizeof(atomic::lifo::link), 8);

				mElementSize = x_intu::alignUp(sizeof(TElement), X_ALIGNMENT_DEFAULT);
				mElementBufferSize = (size+1) * mElementSize;
				mElementBuffer = (xbyte*)_allocator->allocate(mElementBufferSize, X_ALIGNMENT_DEFAULT);
				mElementRefBuffer = (atomic::atom_s32*)_allocator->allocate((size+1) * sizeof(atomic::atom_s32), X_ALIGNMENT_DEFAULT);
				for (u32 i=0; i<=size; ++i)
				{
					mElementRefBuffer[i] = atomic::atom_s32();
				}
				return mQueue.init(mFifo, size+1, mLifo, size+1, mElementBuffer, mElementBufferSize, mElementSize, mElementRefBuffer);
			}

			void					clear(x_iallocator* _allocator)
			{
				mQueue.clear();

				_allocator->deallocate(mFifo);
				_allocator->deallocate(mLifo);
				_allocator->deallocate(mElementBuffer);
				_allocator->deallocate(mElementRefBuffer);

				mFifo = NULL;
				mLifo = NULL;
				mElementBuffer = NULL;
				mElementSize = 0;
				mElementBufferSize = 0;
				mElementRefBuffer = NULL;
			}

			bool					push(TElement const& element, u32& outIndex);
			bool					pop(TElement& element);

			bool					inside(u32 index) const					{ return mQueue.inside(index); }

			bool					empty() const;
			bool					full() const;
			xcore::u32				size() const;

		private:
			atomic::fifo::link*		mFifo;
			atomic::lifo::link*		mLifo;
			xbyte*					mElementBuffer;
			u32						mElementSize;
			u32						mElementBufferSize;
			atomic::atom_s32*		mElementRefBuffer;
			atomic::queue<void*>	mQueue;
		};

		///< Producer only: Adds item to the circular queue. 
		/// If queue is full at 'push' operation no update/overwrite
		/// will happen, it is up to the caller to handle this case
		///
		/// \param element copy by reference the input item
		/// \return whether operation was successful or not
		template<typename TElement>
		bool cqueue<TElement>::push(TElement const& element, u32& outIndex)
		{
			void* ptr = (void*)element;
			bool r = mQueue.push(ptr, outIndex);
			return r;
		}

		/// Consumer only: Removes and returns item from the queue
		/// If queue is empty at 'pop' operation no retrieve will happen
		/// It is up to the caller to handle this case
		///
		/// \param element return by reference the wanted item
		/// \return whether operation was successful or not */
		template<typename TElement>
		bool cqueue<TElement>::pop(TElement& element)
		{
			void* ptr;
			bool r = mQueue.pop(ptr);
			element = (TElement)ptr;
			return r;
		}

		/// Useful for testing and Consumer check of status
		/// Remember that the 'empty' status can change quickly
		/// as the Producer adds more items.
		///
		/// \return true if circular buffer is empty */
		template<typename TElement>
		bool cqueue<TElement>::empty() const
		{
			return mQueue.empty();
		}

		/// Useful for testing and Producer check of status
		/// Remember that the 'full' status can change quickly
		/// as the Consumer catches up.
		///
		/// \return true if circular buffer is full.  */
		template<typename TElement>
		bool cqueue<TElement>::full() const
		{
			return mQueue.room() == 0;
		}

		/// Returns the number of items in the queue
		///
		/// \return size.  */
		template<typename TElement>
		xcore::u32 cqueue<TElement>::size() const
		{
			return mQueue.size();
		}
	}
}

#endif ///< __X_FILESYSTEM_CONCURRENT_QUEUE_H__
