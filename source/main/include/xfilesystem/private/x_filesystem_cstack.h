#ifndef __X_FILESYSTEM_CONCURRENT_STACK_H__
#define __X_FILESYSTEM_CONCURRENT_STACK_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_types.h"
#include "xbase/x_integer.h"
#include "xatomic/x_atomic.h"
#include "xatomic/x_stack.h"
#include "xfilesystem/private/x_filesystem_constants.h"
#include "xfilesystem/private/x_filesystem_constants.h"


namespace xcore
{
	namespace xfilesystem
	{
		template <typename TElement>
		class cstack
		{
		public:
			cstack()				{	}

			XFILESYSTEM_OBJECT_NEW_DELETE()

			bool					init(x_iallocator* _allocator, u32 size)
			{
				return mStack.init(_allocator,size);
			}

			void					clear()
			{
				mStack.clear();
			}

			bool					push(TElement const& element);
			bool					pop(TElement& element);
			bool					empty() const;
			bool					full() const;
			xcore::u32				size() const;

		private:

			atomic::stack<void*>	mStack;
		};

		template <typename TElement>
		bool cstack<TElement>::push(TElement const& element)
		{
			void* ptr = (void*)element;
			bool r = mStack.push(ptr);
			return r;
		}

		template <typename TElement>
		bool cstack<TElement>::pop(TElement& element)
		{
			void* ptr = NULL;
			bool r = mStack.pop(ptr);
			element = (TElement)ptr;
			return r;
		}

		template <typename TElement>
		bool cstack<TElement>::empty() const
		{
			return mStack.empty();
		}

		template <typename TElement>
		bool cstack<TElement>::full() const
		{
			return mStack.room() == 0;
		}

		template <typename TElement>
		xcore::u32 cstack<TElement>::size() const
		{
			return mStack.size();
		}
	}
}
#endif