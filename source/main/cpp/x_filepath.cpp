#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_filepath.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//==============================================================================
		// xfilepath: Device:\\Folder\Folder\Filename.Extension
		//==============================================================================
		/*
		char			*mBuffer;
		s32				mBufferSize;
		s32				mLength;
		*/
		xfilepath::xfilepath()
			: mBuffer(NULL)
			, mBufferSize(0)
			, mLength(0)
		{
		}
		xfilepath::xfilepath(const xfilepath& filepath)
			: mBuffer(filepath.mBuffer)
			, mBufferSize(filepath.mBufferSize)
			, mLength(filepath.mLength)
		{
		}

		xfilepath::xfilepath(char* buffer, s32 bufferSize)
			: mBuffer(buffer)
			, mBufferSize(bufferSize)
			, mLength(0)
		{
		}

		xfilepath::xfilepath(char* buffer, s32 bufferSize, const xfilepath& filepath)
			: mBuffer(buffer)
			, mBufferSize(bufferSize)
			, mLength(0)
		{
			concat(filepath.mBuffer, filepath.mLength);	
		}

		xfilepath::xfilepath(char* buffer, s32 bufferSize, const char* str)
			: mBuffer(buffer)
			, mBufferSize(bufferSize)
			, mLength(0)
		{
			concat(str, x_strlen(str));	
		}

		xfilepath&		xfilepath::operator =  ( const char* str )
		{
			mLength = 0;
			concat(str, x_strlen(str));
			return *this;
		}

		xfilepath&		xfilepath::operator += ( const char* str )
		{
			concat(str, x_strlen(str));
			return *this;
		}

		bool			xfilepath::operator == ( const xfilepath& rhs) const
		{
			return (mLength == rhs.mLength) && x_strCompareNoCase(mBuffer, rhs.mBuffer) == 0;
		}

		bool			xfilepath::operator != ( const xfilepath& rhs) const
		{
			return (mLength != rhs.mLength) || x_strCompareNoCase(mBuffer, rhs.mBuffer) != 0;
		}

		void			xfilepath::concat(const char* str, s32 length)
		{
			char      * dst = &mBuffer[mLength];
			const char* src = str;
			for (s32 i=0; mLength<mBufferSize && i<length; ++i)
			{
				*dst++ = *src++;
				++mLength;
			}
			mBuffer[mLength] = '\0';
		}

		void			xfilepath::fixSlashes(bool UnixStyle)
		{
			for (s32 i=0; i<mLength; ++i)
			{
				char c = mBuffer[i];
				if (UnixStyle)
				{
					if (c == '\\')
						mBuffer[i] = '/';
				}
				else
				{
					if (c == '/')
						mBuffer[i] = '\\';
				}
			}
		}


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
