#ifndef __X_FILESYSTEM_FILEINFO_H__
#define __X_FILESYSTEM_FILEINFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"

#include "xfilesystem\private\x_filesystem_constants.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xfiledevice;
		struct xfileasync;

		struct xfiledata
		{
								xfiledata() 
									: m_nFileIndex(0)
									, m_szFilename(NULL)
									, m_sFilenameMaxLen(0)
								{
									clear();
								}

			XFILESYSTEM_OBJECT_NEW_DELETE()

			u64 				m_uByteSize;

			s32 				m_nFileIndex;
			s32 				m_nLastError;

			char*				m_szFilename;
			s32					m_sFilenameMaxLen;

			xbool				m_boReading;
			xbool				m_boWriting;
			
			xfiledevice*		m_pFileDevice;

			// Async IO worker thread will write here:
			volatile u32 		m_nFileHandle;

			void				clear()
			{
				m_uByteSize			= 0;

				m_nLastError		= 0;

				m_boReading			= true;
				m_boWriting			= false;

				m_pFileDevice		= NULL;

				m_nFileHandle		= (u32)-1;
			}
		};

	};
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_FILEINFO_H__
//==============================================================================
#endif