#ifndef __X_FILESYSTEM_FILEPATH_H__
#define __X_FILESYSTEM_FILEPATH_H__
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
		class xdirpath;

		//==============================================================================
		// xfilepath: 
		//		- Relative:		Folder\Filename.Extension
		//		- Absolute:		Device:\Folder\Folder\Filename.Extension
		//==============================================================================
		class xfilepath
		{
		private:
			enum ESettings { MAX_FILEPATH = 255 };
			char			mStringBuffer[MAX_FILEPATH + 1];
			xcstring		mString;

		public:
							xfilepath();
							xfilepath(const char* str);
							xfilepath(const xfilepath& filepath);
			explicit		xfilepath(const xdirpath& dir, const xfilepath& filename);
							~xfilepath();

			void			clear();

			s32				length() const;
			static s32		sMaxLength()			{ return MAX_FILEPATH; }
			s32				maxLength() const;
			xbool			empty() const;
			xbool			isAbsolute() const;
			const char*		extension() const;
			const char*		relative() const;													///< Points just after device part

			void			setDeviceName(const char* deviceName);
			void			getDeviceName(char* deviceName, s32 deviceNameMaxLength) const;
			void			setDevicePart(const char* devicePart);
			void			getDevicePart(char* devicePart, s32 devicePartMaxLength) const;
			
			xfilepath&		operator =  ( const xfilepath& );

			xfilepath&		operator =  ( const char* );
			xfilepath&		operator += ( const char* );

			bool			operator == ( const xfilepath& rhs) const;
			bool			operator != ( const xfilepath& rhs) const;

			char			operator [] (s32 index) const;

			const char*		c_str() const;

		private:
			void			fixSlashes();														///< Fix slashes, replace '/' with '\'. For Unix, replace '\' with '/'.
		};

		inline xfilepath	operator + (const xdirpath& dir, const xfilepath& filename)	{ return xfilepath(dir, filename); }

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_FILEPATH_H__
//==============================================================================
#endif