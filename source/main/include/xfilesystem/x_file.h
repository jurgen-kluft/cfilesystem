//==============================================================================
//
//  x_stdio.h
//
//==============================================================================
#ifndef __X_STDIO_H__
#define __X_STDIO_H__
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
// xCore namespace
//==============================================================================
namespace xcore
{
	///< Deprecated
	class xfile
	{
	public:
								xfile			(void);
								~xfile			(void);

		xbool                   open            (const char* fileName, const char* mode);
		void                    close           (void);

		xbool                   readRaw         (void* buffer, s32 size, s32 count);
		void                    writeRaw        (const void* buffer, s32 size, s32 count);
		void                    toMemory        (void* buffer, s32 bufferSize);
		xbool				    synchronize     (xbool isBlock);
		void                    asyncAbort      (void);
		s32                     printf          (const char* formatStr, const x_va_list& args);
		void                    forceFlush      (xbool isOnOff);
		void                    flush           (void);
		void                    seekOrigin      (s32 offset);
		void                    seekEnd         (s32 offset);
		void                    seekCurrent     (s32 offset);
		s32                     tell            (void);
		xbool                   isEof           (void);
		s32                     getC            (void);
		void                    putC            (s32 inC, s32 count = 1, xbool isUpdatePos = xTRUE);
		void                    alignPutC       (s32 inC, s32 count = 0, s32 aligment = 4, xbool isUpdatePos = xTRUE);
		s32                     getFileLength   (void);
		xbool            		getFileTime     (u64& fileTime);
		xbool            		deleteFile      (void);

		template<class T> void  write           (T& val);
		template<class T> void  write           (T* val, s32 count);
		template<class T> xbool read            (T& val);
		template<class T> xbool read            (T* val, s32 count);

	private:
		void					clear			(void);

		u32						mFile;
		u32						mFileLength;
		u32						mPos;
		xbool					mRead;
		xbool					mWrite;
		xbool					mAsync;
	};


	//==============================================================================
	// INLINES
	//==============================================================================
	#include "private\x_file_inline.h"

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

//==============================================================================
// END __X_STDIO_H__
//==============================================================================
#endif
