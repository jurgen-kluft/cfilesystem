#ifndef __X_FILESYSTEM_FILE_DIR_ATTRIBUTES_H__
#define __X_FILESYSTEM_FILE_DIR_ATTRIBUTES_H__
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
		struct xattributes
		{
									xattributes();
									xattributes(const xattributes&);
									xattributes(bool boArchive, bool boReadonly, bool boHidden, bool boSystem);

			bool					isArchive() const;
			bool					isReadOnly() const;
			bool					isHidden() const;
			bool					isSystem() const;

			void					setArchive(bool);
			void					setReadOnly(bool);
			void					setHidden(bool);
			void					setSystem(bool);

			xattributes&			operator = (const xattributes&);

			bool					operator == (const xattributes&) const;
			bool					operator != (const xattributes&) const;

		private:
			xcore::u32				mFlags;
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
// END __X_FILESYSTEM_FILE_DIR_ATTRIBUTES_H__
//==============================================================================
#endif
