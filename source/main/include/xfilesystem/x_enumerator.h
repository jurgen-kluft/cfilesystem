#ifndef __X_FILESYSTEM_ENUMERATOR_H__
#define __X_FILESYSTEM_ENUMERATOR_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif


namespace xcore
{
	struct enumerate_delegate
	{
		// When you receive a 'dirinfo' and return true you indicate that you want to
		// recurse into that directory. When returning 'false' you indicate that do not
		// want to recurse into that directory.
		// When you receive a 'fileinfo' and return false you indicate that you want to
		// terminate the iteration.
		virtual bool operator () (s32 depth, const fileinfo* fi, const dirinfo* di) = 0;
	};

};

#endif
