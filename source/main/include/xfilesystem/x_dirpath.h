#ifndef __X_FILESYSTEM_DIRPATH_H__
#define __X_FILESYSTEM_DIRPATH_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xpath;
		class xfilepath;

		//==============================================================================
		// xdirpath: 
		//		- Relative:		FolderA\FolderB\
		//		- Absolute:		Device:\FolderA\FolderB\
		//==============================================================================
		class xdirpath
		{
		protected:
			friend class xfilepath;
			char			mStringBuffer[256];
			xcstring		mString;

		public:
							xdirpath();
							xdirpath(const char* str);
							xdirpath(const xdirpath& dir);
							~xdirpath();

			void			clear();

			s32				length() const;
			s32				maxLength() const;
			xbool			empty() const;
			xbool			isAbsolute() const;
			const char*		relative() const;

			void			setDeviceName(const char* deviceName);
			void			getDeviceName(char* deviceName, s32 deviceNameMaxLength) const;
			void			setDevicePart(const char* devicePart);
			void			getDevicePart(char* devicePart, s32 devicePartMaxLength) const;
			
			xdirpath&		operator =  ( const xdirpath& );

			xdirpath&		operator =  ( const char* );
			xdirpath&		operator += ( const char* );

			bool			operator == ( const xdirpath& rhs) const;
			bool			operator != ( const xdirpath& rhs) const;

			char			operator [] (s32 index) const;

			const char*		c_str() const;

		private:
			void			fixSlashes();									///< Fix slashes, replace '/' with '\'. For Unix, replace '\' with '/'.
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
// END __X_FILESYSTEM_DIRPATH_H__
//==============================================================================
#endif