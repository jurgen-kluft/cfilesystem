#ifndef __X_FILEPATH_H__
#define __X_FILEPATH_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//==============================================================================
		// xfilepath: Device:\\Folder\Folder\Filename.Extension
		//==============================================================================
		class xfilepath
		{
		private:
			char			*mBuffer;
			s32				mBufferSize;
			s32				mLength;

		public:
							xfilepath();
							xfilepath(const xfilepath& filepath);
							xfilepath(char* buffer, s32 bufferSize);
							xfilepath(char* buffer, s32 bufferSize, const xfilepath& filepath);
							xfilepath(char* buffer, s32 bufferSize, const char* str);

			void			clear();

			s32				length() const;
			s32				maxLength() const;
			xbool			empty() const;

			xfilepath&		operator =  ( const char* str );
			xfilepath&		operator += ( const char* str );

			bool			operator == ( const xfilepath& rhs) const;
			bool			operator != ( const xfilepath& rhs) const;

			char&			operator [] (s32 index);
			char			operator [] (s32 index) const;

			const char*		c_str() const;

		private:
			void			concat(const char* str, s32 length);
			void			zeroTerminate();
			void			fixSlashes(bool UnixStyle);					///< Fix slashes, replace '/' with '\'. For Unix, replace '\' with '/'.
		};

		// Inline functions
		#include "xfilesystem\private\x_filepath_inline.h"

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILEPATH_H__
//==============================================================================
#endif