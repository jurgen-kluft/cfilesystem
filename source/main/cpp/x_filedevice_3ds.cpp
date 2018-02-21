#include "xbase/x_target.h"
#ifdef TARGET_3DS

//==============================================================================
// INCLUDES
//==============================================================================


#include "xbase/x_debug.h"
#include "xbase/x_limits.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_3ds.h"
#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/private/x_filedata.h"
#include "xfilesystem/private/x_fileasync.h"

namespace xcore
{

	//------------------------------------------------------------------------------------------
	//---------------------------------- Nintendo 3DS IO Functions -----------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		class FileDevice_3DS_System : public xfiledevice
		{
			xbool					mCanWrite;

		public:
									FileDevice_3DS_System(xbool boCanWrite)
										: mCanWrite(boCanWrite)							{ }
			virtual					~FileDevice_3DS_System()							{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return mCanWrite; }

			virtual bool			openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle);
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength);
			virtual bool			lengthOfFile(u32 nFileHandle, u64& outLength) const;
			virtual bool			closeFile(u32 nFileHandle);
			virtual bool			deleteFile(u32 nFileHandle, const char* szFilename);
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten);

			enum ESeekMode { __SEEK_ORIGIN = 1, __SEEK_CURRENT = 2, __SEEK_END = 3, };
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos);
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos);

			XFILESYSTEM_OBJECT_NEW_DELETE()
		};

		xfiledevice*		x_CreateFileDevice3DS(xbool boCanWrite)
		{
			xfiledevice* file_device = new FileDevice_3DS_System(boCanWrite);
			return device;
		}

		void			x_DestroyFileDevice3DS(xfiledevice* device)
		{
			FileDevice_3DS_System* file_device = (FileDevice_3DS_System*)device;
			delete file_device;
		}
		
		bool FileDevice_3DS_System::openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle)
		{
			s32	nFlags;
			if(boWrite)
			{
				nFlags	= CELL_FS_O_CREAT | CELL_FS_O_RDWR;
			}
			else
			{
				nFlags	= CELL_FS_O_RDONLY;
			}

			s32	hFile	= INVALID_FILE_HANDLE;
			s32	nResult = cellFsOpen (szFilename, nFlags, &hFile, NULL, 0);
			nFileHandle = (u32)hFile;

			bool boSuccess = (nResult == CELL_OK);
			return boSuccess;
		}

		bool FileDevice_3DS_System::setLengthOfFile(u32 nFileHandle, u64 inLength)
		{
			return false;
		}

		bool FileDevice_3DS_System::lengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			CellFsStat stats;
			CellFsErrno nResult = cellFsStat(szFilename, &stats);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
				outLength = stats.st_size;
			else
				outLength = 0;

			return boSuccess;
		}

		bool FileDevice_3DS_System::closeFile(u32 nFileHandle)
		{
			s32 nResult = cellFsClose (nFileHandle);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				nFileHandle = (u32)INVALID_FILE_HANDLE;
				boSuccess = true;
			}
			return boSuccess;
		}

		bool FileDevice_3DS_System::deleteFile(u32 nFileHandle, const char* szFilename)
		{
			s32 nResult = cellFsUnlink(szFilename);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				nFileHandle = (u32)INVALID_FILE_HANDLE;
				boSuccess = true;
			}
			return boSuccess;
		}

		bool FileDevice_3DS_System::readFile(u32 nFileHandle, void* buffer, u64 pos, u64 count, u64& outNumBytesRead)
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				u64 numBytesRead;
				s32 nResult = cellFsRead (nFileHandle, buffer, count, &numBytesRead);

				bool boSuccess = (nResult == CELL_OK);
				if (boSuccess)
				{
					outNumBytesRead = (u32)numBytesRead;
				}
				else
				{ 
					outNumBytesRead = 0;
				}
				return boSuccess;
			}
			return false;
		}
		bool FileDevice_3DS_System::writeFile(u32 nFileHandle, const void* buffer, u64 pos, u64 count, u64& outNumBytesWritten)
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				u64 numBytesWritten;
				s32 nResult = cellFsWrite (nFileHandle, buffer, count, &numBytesWritten);

				bool boSuccess = (nResult == CELL_OK);
				if (boSuccess)
				{
					outNumBytesWritten = (u32)numBytesWritten;
				}
				else
				{ 
					outNumBytesWritten = 0;
				}
				return boSuccess;
			}
			return false;
		}

		bool FileDevice_3DS_System::seek(u32 nFileHandle, E3dsSeekMode mode, u64 pos, u64& newPos)
		{
			u64	nPos;
			s32 nResult = cellFsLseek (nFileHandle, pos, mode, &nPos);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				newPos = nPos;
			}
			else
			{ 
				newPos = pos;
			}
			return boSuccess;
		}
		bool	FileDevice_3DS_System::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_3DS_System::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_3DS_System::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_END, pos, newPos);
		}

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_3DS
