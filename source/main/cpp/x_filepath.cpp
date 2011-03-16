#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_alias.h"
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
			: mString(NULL)
			, mBuffer(NULL)
			, mBufferSize(0)
			, mLength(0)
		{
		}
		xfilepath::xfilepath(const xfilepath& filepath)
			: mString(filepath.mBuffer)
			, mBuffer(filepath.mBuffer)
			, mBufferSize(filepath.mBufferSize)
			, mLength(filepath.mLength)
		{
		}

		xfilepath::xfilepath(const char* str)
			: mString(str)
			, mBuffer(NULL)
			, mBufferSize(0)
			, mLength(x_strlen(str))
		{
			mLength = x_strlen(str);
		}

		xfilepath::xfilepath(char* buffer, s32 bufferSize, const xfilepath& filepath)
			: mString(buffer)
			, mBuffer(buffer)
			, mBufferSize(bufferSize)
			, mLength(0)
		{
			zeroTerminate();
			if (filepath.mString!=NULL)
				concat(filepath.mString, filepath.mLength);	
			else
				concat(filepath.mBuffer, filepath.mLength);	
		}

		xfilepath::xfilepath(char* buffer, s32 bufferSize, const char* str)
			: mString(buffer)
			, mBuffer(buffer)
			, mBufferSize(bufferSize)
			, mLength(0)
		{
			zeroTerminate();
			concat(str, x_strlen(str));	
		}

		s32				xfilepath::find (const char* str) const
		{
			const char* pos = x_strFind(mString, str);
			if (pos == NULL)
				return -1;
			return reinterpret_cast<s32>(pos) - reinterpret_cast<s32>(mString);
		}

		void			xfilepath::replace(s32 pos, s32 count, const char* str)
		{
			if (mBuffer == NULL)
				return;

			if ((pos + count) < mLength)
			{
				const s32 len1 = count;
				const s32 len2 = str!=NULL ? (s32)x_strlen(str) : 0;

				if (len1 >= len2)
				{
					// Filename becomes shorter since replacement is shorter
					const char* end = mBuffer + mLength;
					const char* src = mBuffer + pos + count;

					char* dst = mBuffer + pos + len2;
					while (src <= end)
						*dst++ = *src++;
				}
				else
				{
					// Filename becomes longer since replacement is longer
					char* end1 = mBuffer + mLength;
					char* end2 = mBuffer + mLength + (len2 - len1);

					// Guard against maximum string length of mBuffer
					if ((mLength + (len2 - len1)) >= mBufferSize)
						return;

					const char* pos1 = mBuffer + pos + count;
					while (end1 >= pos1)
						*end2-- = *end1--;
				}

				if (str!=NULL)
				{
					const char* src = str;
					char* dst = mBuffer + pos;
					while (*src != '\0')
						*dst++ = *src++;
				}

				mLength = x_strlen(mBuffer);
			}
		}

		void			xfilepath::subString(s32 pos, s32 count, char* str, s32 maxStrLen) const
		{
			const char* src = mString + pos;
			const char* send = mString + pos + count;
			char* dst = str;
			char* dend = str + ((count < maxStrLen) ? count : maxStrLen);
			while (src < send)
				*dst++ = *src++;
			*dst++ = '\0';
		}

		xfilepath&		xfilepath::operator =  ( const char* str )
		{
			if (mBuffer != NULL)
			{
				mLength = 0;
				concat(str, x_strlen(str));
			}
			return *this;
		}

		xfilepath&		xfilepath::operator += ( const char* str )
		{
			if (mBuffer != NULL)
			{
				concat(str, x_strlen(str));
			}
			return *this;
		}

		bool			xfilepath::operator == ( const xfilepath& rhs) const
		{
			return (mLength == rhs.mLength) && x_strCompareNoCase(mString, rhs.mString) == 0;
		}

		bool			xfilepath::operator != ( const xfilepath& rhs) const
		{
			return (mLength != rhs.mLength) || x_strCompareNoCase(mString, rhs.mString) != 0;
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
