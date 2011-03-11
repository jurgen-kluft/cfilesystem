#ifndef __XFILESYSTEM_XFILESTREAM_H__
#define __XFILESYSTEM_XFILESTREAM_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xfilesystem\x_stream.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		// Forward declares
		class xfilepath;

		class xfilestream : public xstream
		{
									xfilestream		(const xfilestream&);
			xfilestream&			operator =		(const xfilestream&);

		public:
									xfilestream		(const xfilepath& filename, EFileMode mode, EFileAccess access);
									~xfilestream	(void);
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
// END __XFILESYSTEM_XFILESTREAM_H__
//==============================================================================
#endif
