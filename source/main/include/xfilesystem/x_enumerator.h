#ifndef __X_FILESYSTEM_ENUMERATOR_H__
#define __X_FILESYSTEM_ENUMERATOR_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		template<typename _Arg>
		struct enumerate_delegate
		{
			///< Return false to terminate the breadth first traversal
			virtual void operator () (s32 depth, const _Arg& inf, bool& terminate) = 0;
		protected:
			virtual ~enumerate_delegate() {}
		};

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_ENUMERATOR_H__
//==============================================================================
#endif
