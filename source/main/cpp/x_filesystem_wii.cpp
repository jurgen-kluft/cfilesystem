#include "xbase\x_target.h"
#ifdef TARGET_WII

#include <revolution.h>
#include <revolution\hio2.h>
#include <revolution\nand.h>
#include <revolution\dvd.h>

//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_wii.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		enum EDeviceType
		{	
			FS_WII_DEVICE_DVD  = 1,
			FS_WII_DEVICE_NAND = 2,
			FS_WII_DEVICE_HIO2 = 3,
		};

		struct WiiFileInfo
		{
			EDeviceType			mDeviceType;

			volatile xbool		mAsyncDone;
			volatile s32		mAsyncResult;
		};

		static DVDFileInfo		mDvdFileInfo[FS_MAX_OPENED_FILES];
		static NANDFileInfo		mNandFileInfo[FS_MAX_OPENED_FILES];
		static WiiFileInfo		m_WiiFileInfo[FS_MAX_OPENED_FILES];

		static WiiFileInfo*		GetWiiFileInfo(u32 uHandle)						{ return &m_WiiFileInfo[uHandle]; }
		static DVDFileInfo*		GetDvdFileInfo(u32 uHandle)						{ return &mDvdFileInfo[uHandle]; }
		static NANDFileInfo*	GetNandFileInfo(u32 uHandle)					{ return &mNandFileInfo[uHandle]; }

		static void				DvdCallback(long result, DVDFileInfo* fileInfo)
		{
			WiiFileInfo* wiiInfo = (WiiFileInfo*)DVDGetUserData((DVDCommandBlock*)fileInfo);
			wiiInfo->mAsyncDone = xTRUE;
			wiiInfo->mAsyncResult = result;
		}

		//------------------------------------------------------------------

		static xbool IsDvdInUse( void )
		{
			s32 dvdStatus = DVDGetDriveStatus();

			xbool bError = xFALSE;
			xbool bBusy = xFALSE;
			switch (dvdStatus)
			{
			case DVD_STATE_FATAL_ERROR	:										///< Fatal error occurred.
				bError = xTRUE;
				break;
			case DVD_STATE_END			:										///< Request complete or no request.
			case DVD_STATE_BUSY			:										///< Request is currently processing.
				bBusy = xTRUE;
				break;
			case DVD_STATE_NO_DISK		:										///< No disk in the drive.
				bError = xTRUE;
				break;
			case DVD_STATE_WRONG_DISK	:										///< Wrong disk in the drive.
				bError = xTRUE;
				break;
			case DVD_STATE_MOTOR_STOPPED:										///< Motor stopped.
			case DVD_STATE_PAUSING		:										///< The device driver is paused using the DVD Pause function.
				break;
			case DVD_STATE_RETRY		:										///< Retry error occurred.
				bError = xTRUE;
				break;
			}

			return bBusy;
		}
	};

	//------------------------------------------------------------------------------------------
	//---------------------------------- WII IO Functions --------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		enum EWiiSeekMode
		{
			__SEEK_ORIGIN = 1,
			__SEEK_CURRENT = 2,
			__SEEK_END = 3,
		};

		//------------------------------------------------------------------------------------------
		//---------------------------------- HIO2 --------------------------------------------------
		//------------------------------------------------------------------------------------------
		namespace xwii_hio2
		{
			//Globals
			const s32			FS_HIO2_SHARE_SIZE = 0x2000;				///< Share memory size 8KByte
			const s32			FS_HIO2_SHARE_ADDR = 0x00000000;			///< Share memory addresse

			xbool				m_Connected = xFALSE;
			xbool				m_PingReceived = xFALSE;
			xbool				m_MailWaiting = xFALSE;
			xbool				m_Hio2Initialized = xFALSE;
			HIO2DeviceType		m_UsbType = HIO2_DEVICE_INVALID;
			HIO2Handle			m_UsbHandle = HIO2_INVALID_HANDLE_VALUE;
			s32					m_UsbChannel = -1;

			xbyte				m_AlignedBuffer[512+31];
			xbyte*				m_TempBuffer = NULL;

			//------------------------------------------------
			//       Mailbox bit define                      |
			// +-----+-------+-------------------------------+
			// |31 24|23   16|15                            0|
			// +-----+-------+-------------------------------+
			// | ID  |  CMD  |           CMD Option          |
			// +-----+-------+-------------------------------+
			//
			#define	FS_MAIL_ID_SHIFT					24
			#define	FS_MAIL_ID_MASK						0xFF000000
			#define	FS_MAIL_CMD_SHIFT					16
			#define	FS_MAIL_CMD_MASK					0x00FF0000
			#define	FS_MAIL_OPTION_SHIFT				0
			#define	FS_MAIL_OPTION_MASK					0x0000FFFF
			#define	FS_MAIL_GET_CMD( mail )				(((mail)  & FS_MAIL_CMD_MASK) >> FS_MAIL_CMD_SHIFT )
			#define	FS_MAIL_GET_OPTION( mail )			(((mail)  & FS_MAIL_OPTION_MASK) >> FS_MAIL_OPTION_SHIFT )
			#define	FS_MAIL_MAKECMD( id, cmd, option) 	((((u32)(id) << FS_MAIL_ID_SHIFT) & FS_MAIL_ID_MASK) | (((u32)(cmd) << FS_MAIL_CMD_SHIFT) & FS_MAIL_CMD_MASK) | (((u32)(option) & FS_MAIL_OPTION_MASK) << FS_MAIL_OPTION_SHIFT))

			// Cmd define
			#define	FS_CMD_NONE						0x00						///< Nop
			#define	FS_CMD_PING						0x01						///< Ping
			#define	FS_CMD_FILE_OPEN				0x02						///< 
			#define	FS_CMD_FILE_CLOSE				0x03						///< 
			#define	FS_CMD_FILE_READ				0x04						///< 
			#define	FS_CMD_FILE_WRITE				0x05						///< 
			#define	FS_CMD_FILE_SEEK				0x06						///< 
			#define	FS_CMD_FILE_DELETE				0x07						///< 
			#define	FS_CMD_FILE_LENGTH				0x08						///< 
			#define	FS_CMD_ACK						0x10						///< 

			// Option
			#define FS_OPTION_READ					0x01
			#define FS_OPTION_WRITE					0x02

			//------------------------------------------------------------------------------------------
			//---------------------------------- Callbacks ---------------------------------------------
			//------------------------------------------------------------------------------------------
			static BOOL __HostIOEnumCallback( HIO2DeviceType type )
			{
				m_UsbType = type;
				return( xFALSE );
			}

			static void __ReceiveMailCallback( HIO2Handle handle )
			{
				(void) handle;

				//Mail is waiting for us - don't look at it yet
				m_MailWaiting = xTRUE;
			}

			static void __DisconnectCallback( HIO2Handle handle )
			{
				(void) handle;
				m_Connected = xFALSE;
			}

			//------------------------------------------------------------------------------------------
			//---------------------------------- Initialization ----------------------------------------
			//------------------------------------------------------------------------------------------
#ifndef USING_WII_PROFILER
			static xbool __ConnectToUSB( void )
			{
				if( !m_Connected )
				{
					if( HIO2EnumDevices( __HostIOEnumCallback ) == TRUE )
					{
						if( m_UsbType != HIO2_DEVICE_INVALID )
						{
							m_UsbHandle = HIO2Open( m_UsbType, __ReceiveMailCallback, __DisconnectCallback );
							if( m_UsbHandle != HIO2_INVALID_HANDLE_VALUE )
							{
								u32 temp = 0;
								HIO2ReadMailbox( m_UsbHandle, (unsigned long*)&temp );
								m_Connected = xTRUE;
							}
						}
					}
				}

				return( m_Connected );
			}

			static xbool __Connect()
			{

				static HIO2Error hioerror(HIO2_ERROR_NONE);

				ASSERT( (hioerror = HIO2GetLastError()) == HIO2_ERROR_NONE );

				if( !m_Hio2Initialized )
				{
					if (HIO2Init() == TRUE)
					{
						x_printf ("Stdio:"TARGET_PLATFORM_STR" INFO HIO2 initialized\n");
						m_Hio2Initialized = xTRUE;
					}
				}

				if( !m_Connected )
				{
					__ConnectToUSB();
					if (m_Connected)
					{
						x_printf ("Stdio:"TARGET_PLATFORM_STR" INFO HIO2 connection (re)established\n");
					}
				}
				return m_Hio2Initialized && m_Connected;
			}

			static xbool __Init()
			{
				m_TempBuffer = (xbyte*)(((u32)m_AlignedBuffer + 31) & ~(31));
				return __Connect();
			}

			static void __ReadMail(u32& outMail)
			{
				while (!m_MailWaiting) { __Connect(); }
				m_MailWaiting = xFALSE;
				while( !HIO2ReadMailbox( m_UsbHandle, (unsigned long*)&outMail ) ) { __Connect(); }
			}

			static void __ReadMailContent(xbyte* buffer, u32 bytesToRead)
			{
				while( !HIO2Read( m_UsbHandle, FS_HIO2_SHARE_ADDR, buffer, bytesToRead ) ) { __Connect(); }
			}

			static void __WriteMail(u32 mail)
			{
				while( !HIO2WriteMailbox( m_UsbHandle, mail ) ) { __Connect(); }
			}

			static void __WriteMailContent(const xbyte* buffer, u32 bytesToWrite)
			{
				DCFlushRange( (void*)buffer, bytesToWrite );
				while( !HIO2Write( m_UsbHandle, FS_HIO2_SHARE_ADDR, buffer, bytesToWrite ) ) { __Connect(); }
			}

			static bool __OpenOrCreateFile(FileInfo* pInfo)
			{
				__Connect();

				xstring_buffer str(m_TempBuffer, FS_MAX_PATH);
				str.copy(pInfo->m_szFilename);

				__WriteMailContent(m_TempBuffer, x_Align(str.getLength(), 32));
				__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_OPEN, pInfo->m_boWriting ? FS_OPTION_WRITE : FS_OPTION_READ) );
				
				u32 mail;
				__ReadMail(mail);
				s32 cmd = FS_MAIL_GET_CMD(mail);								// Received mail should be an ACK cmd
				xbool boSuccess = (cmd == FS_CMD_ACK);
				return boSuccess;
			}

			static bool __LengthOfFile(FileInfo* pInfo, u64& outLength)
			{
				__Connect();

				__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_LENGTH, 0) );

				u32 mail;
				__ReadMail(mail);

				// Received mail should be an ACK cmd
				s32 cmd = FS_MAIL_GET_CMD(mail);								// Received mail should be an ACK cmd
				xbool boSuccess = (cmd == FS_CMD_ACK);

				if (!boSuccess)
				{
					outLength = 0;
				}
				else
				{
					__ReadMailContent((xbyte*)m_TempBuffer, 32);
					outLength = *((u64*)m_TempBuffer);
				}

				return boSuccess;
			}

			static bool __CloseFile(FileInfo* pInfo)
			{
				__Connect();

				xbool boSuccess = xTRUE;

				xstring_buffer str(m_TempBuffer, FS_MAX_PATH);
				str.copy(pInfo->m_szFilename);

				__WriteMailContent(m_TempBuffer, x_Align(str.getLength(), 32));
				__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_CLOSE, 0) );

				u32 mail;
				__ReadMail(mail);
				s32 cmd = FS_MAIL_GET_CMD(mail);
				if (cmd != FS_CMD_ACK)											// Received mail should be an ACK cmd
				{
					boSuccess = xFALSE;
				}

				return boSuccess;
			}


			static bool __DeleteFile(FileInfo* pInfo)
			{
				__Connect();

				xbool boSuccess = xTRUE;

				xstring_buffer str(m_TempBuffer, FS_MAX_PATH);
				str.copy(pInfo->m_szFilename);

				__WriteMailContent(m_TempBuffer, x_Align(str.getLength(), 32));
				__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_DELETE, 0) );

				u32 mail;
				__ReadMail(mail);

				s32 cmd = FS_MAIL_GET_CMD(mail);
				if (cmd != FS_CMD_ACK)											// Received mail should be an ACK cmd
				{
					boSuccess = xFALSE;
				}
				return boSuccess;
			}

			static bool __ReadFile(FileInfo* pInfo, void* buffer, s32 size, u64& outNumBytesRead)
			{
				__Connect();

				xbool boSuccess = xTRUE;

				xbyte* data = (xbyte*)buffer;
				s32 chunkOffset = 0;
				while (chunkOffset <= size)
				{
					s32 chunkSize = (size - chunkOffset) >= FS_HIO2_SHARE_SIZE ? FS_HIO2_SHARE_SIZE : 0;
					if (chunkSize==0)
					{
						chunkSize = x_AlignLower(size - chunkOffset, 32);
					}

					// Send mail to request a read
					s32 offsetAndChunkSize[2];
					offsetAndChunkSize[0] = chunkOffset;
					offsetAndChunkSize[1] = chunkSize;
					__WriteMailContent((xbyte*)offsetAndChunkSize, sizeof(offsetAndChunkSize));
					__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_READ, 0) );

					u32 mail;
					__ReadMail(mail);
					s32 cmd = FS_MAIL_GET_CMD(mail);
					if (cmd != FS_CMD_ACK)										// Received mail should be an ACK cmd
					{
						boSuccess = xFALSE;
						break;
					}

					if (chunkSize!=0)
					{
						__ReadMailContent(data + chunkOffset, chunkSize);
					}
					else
					{
						if (chunkSize > 32)
						{
							__ReadMailContent(data + chunkOffset, chunkSize);
						}
						else
						{
							__ReadMailContent(m_TempBuffer, 32);
							for (s32 i=0; i<chunkSize; ++i)
								data[chunkOffset + i] = m_TempBuffer[i];
						}
					}
					chunkOffset     += chunkSize;
					outNumBytesRead += chunkSize;

					// Send mail that the read has been done
					__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_ACK, FS_CMD_FILE_READ) );
					
					__ReadMail(mail);
					cmd = FS_MAIL_GET_CMD(mail);
					if (cmd != FS_CMD_ACK)										// Received mail should be an ACK cmd
					{
						boSuccess = xFALSE;
						break;
					}
				}

				DCFlushRange( buffer, size );

				return boSuccess;
			}

			static bool __WriteFile(FileInfo* pInfo, const void* buffer, s32 count, u64& outNumBytesWritten)
			{
				__Connect();
				
				DCFlushRange( (void*)buffer, count );

				xbyte const * data = static_cast<xbyte const *>(buffer);

				while( count > 0 )
				{
					u32 chunk_size ( count >= FS_HIO2_SHARE_SIZE ? FS_HIO2_SHARE_SIZE : (count >= 32 ? 32 : count) );

					if(chunk_size < 32)
					{
						// This line is only for shading the temporary buffer
						xcore::x_memset(reinterpret_cast<void*>(m_TempBuffer), 'X', 32);
						xcore::x_memcpy(reinterpret_cast<void*>(m_TempBuffer), data, count);
					}

					xbyte const * chunk ( chunk_size < 32 ? m_TempBuffer : data );

					__WriteMailContent( chunk, chunk_size < 32 ? 32 : chunk_size );

					data  += chunk_size;
					count -= chunk_size;

					__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_WRITE, chunk_size) );

					u32 mail;
					__ReadMail(mail);

					s32 cmd = FS_MAIL_GET_CMD(mail);

					if (cmd != FS_CMD_ACK)										// Received mail should be an ACK cmd
						return xFALSE;
				}

				// semantic of this piece of code is wrong
				//s32 chunkOffset = 0;
				//while (chunkOffset <= count)
				//{
				//	s32 chunkSize = (count - chunkOffset) >= FS_HIO2_SHARE_SIZE ? FS_HIO2_SHARE_SIZE : 0;
				//	if (chunkSize!=0)
				//	{
				//		__WriteMailContent(data + chunkOffset, chunkSize);
				//	}
				//	else
				//	{
				//		chunkSize = x_AlignLower(count - chunkOffset, 32);
				//		if (chunkSize >= 32)
				//		{
				//			__WriteMailContent(data + chunkOffset, chunkSize);
				//		}
				//		else
				//		{
				//			for (s32 i=0; i<chunkSize; ++i)
				//				m_TempBuffer[i] = data[chunkOffset + i];
				//			__WriteMailContent(m_TempBuffer, 32);
				//			chunkSize = 32;
				//		}
				//	}
				//	chunkOffset += chunkSize;
				//	
				//	__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_WRITE, chunkSize) );

				//	u32 mail;
				//	__ReadMail(mail);

				//	s32 cmd = FS_MAIL_GET_CMD(mail);
				//	if (cmd != FS_CMD_ACK)										// Received mail should be an ACK cmd
				//	{
				//		boSuccess = xFALSE;
				//		break;
				//	}
				//}

				return xTRUE;
			}

			static bool __Seek(FileInfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos)
			{
				__Connect();

				u64* buffer = (u64*)m_TempBuffer;
				*buffer = pos;
				__WriteMailContent(m_TempBuffer, 32);
				__WriteMail( FS_MAIL_MAKECMD(pInfo->m_nFileIndex, FS_CMD_FILE_SEEK, 0) );

				u32 mail;
				__ReadMail(mail);

				s32 cmd = FS_MAIL_GET_CMD(mail);								// Received mail should be an ACK cmd
				xbool boSuccess = (cmd == FS_CMD_ACK);
				return boSuccess;
			}
#else // USING_WII_PROFILER
			static xbool __Init()
			{
				return xTRUE;
			}

			static void __ReadMail(u32&)
			{
			}

			static void __ReadMailContent(xbyte*, u32)
			{
			}

			static void __WriteMail(u32 mail)
			{
			}

			static void __WriteMailContent(const xbyte*, u32)
			{
			}

			static bool __OpenOrCreateFile(FileInfo*)
			{
				return xTRUE;
			}

			static bool __LengthOfFile(FileInfo*, u64&)
			{
				return xTRUE;
			}

			static bool __CloseFile(FileInfo*)
			{
				return xTRUE;
			}

			static bool __DeleteFile(FileInfo*)
			{
				return xTRUE;
			}

			static bool __ReadFile(FileInfo*, void*, s32, u64&)
			{
				return xTRUE;
			}

			static bool __WriteFile(FileInfo*, const void*, s32, u64&)
			{
				return xTRUE;
			}

			static bool __Seek(FileInfo*, EWiiSeekMode, u64, u64&)
			{
				return xTRUE;
			}
#endif// USING_WII_PROFILER
		};

		//------------------------------------------------------------------------------------------
		//---------------------------------- NAND ---------------------------------------------------
		//------------------------------------------------------------------------------------------
		namespace xwii_nand
		{
			namespace __private
			{
				static GXColor sErrorPageBGColor = {   0,   0,   0, 0 };
				static GXColor sErrorPageFGColor = { 255, 255, 255, 0 };

				static void __CheckNANDError(s32 errorCode)
				{
					switch (errorCode)
					{
					case NAND_RESULT_UNKNOWN:
						OSFatal(sErrorPageFGColor, sErrorPageBGColor, "NAND_RESULT_UNKNOWN");
						break;
					case NAND_RESULT_BUSY:
					case NAND_RESULT_ALLOC_FAILED:
						OSFatal(sErrorPageFGColor, sErrorPageBGColor, "NAND_RESULT_BUSY || NAND_RESULT_ALLOC_FAILED");
						break;
					case NAND_RESULT_CORRUPT:
						OSFatal(sErrorPageFGColor, sErrorPageBGColor, "NAND_RESULT_CORRUPT");
						break;
					}
				}
			};

			static bool __Init()
			{
				return (NANDInit() == NAND_RESULT_OK);
			}

			static bool __OpenOrCreateFile(FileInfo* pInfo)
			{
				NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);
				pInfo->m_nFileHandle = (u32)fileInfo;

				u8 accType = pInfo->m_boWriting ? NAND_ACCESS_RW : NAND_ACCESS_READ;

				s32 result = NANDOpen(pInfo->m_szFilename, fileInfo, accType);
				__private::__CheckNANDError(result);
				
				if (result == NAND_RESULT_NOEXISTS && pInfo->m_boWriting)
				{
					result = NANDCreate(pInfo->m_szFilename, NAND_PERM_OWNER_MASK, accType);
					__private::__CheckNANDError(result);

					if (result == NAND_RESULT_OK)
					{
						result = NANDOpen(pInfo->m_szFilename, fileInfo, accType);
						__private::__CheckNANDError(result);
					}
				}

				return (result == NAND_RESULT_OK);
			}

			static bool __LengthOfFile(FileInfo* pInfo, u64& outLength)
			{
				NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

				unsigned long length = 0;
				s32 result = NANDGetLength(fileInfo, &length);
				__private::__CheckNANDError(result);

				outLength = (u64)length;

				return (result == NAND_RESULT_OK);
			}

			static bool __CloseFile(FileInfo* pInfo)
			{
				NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

				s32 result = NANDClose(fileInfo);
				__private::__CheckNANDError(result);

				pInfo->m_nFileHandle = INVALID_FILE_HANDLE;

				return (result == NAND_RESULT_OK);
			}

			static bool __DeleteFile(FileInfo* pInfo)
			{
				s32 result = NANDDelete(pInfo->m_szFilename);
				__private::__CheckNANDError(result);

				return (result == NAND_RESULT_OK);
			}

			static bool __ReadFile(FileInfo* pInfo, void* buffer, s32 size, u64& outNumBytesRead)
			{
				NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

				outNumBytesRead = 0;

				// make sure size is multiple of 32
				if (size % 32 == 0)
				{
					s32 result = NANDRead(fileInfo, buffer, (u32)size);
					__private::__CheckNANDError(result);

					if (result >= 0)
						outNumBytesRead = (u64)result;
				}

				return (outNumBytesRead != 0);
			}

			static bool __WriteFile(FileInfo* pInfo, const void* buffer, s32 size, u64& outNumBytesWritten)
			{
				NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

				outNumBytesWritten = 0;

				// make sure size is multiple of 32
				if (size % 32 == 0)
				{
					s32 result = NANDWrite(fileInfo, buffer, (u32)size);
					__private::__CheckNANDError(result);

					if (result >= 0)
						outNumBytesWritten = (u64)result;
				}

				return (outNumBytesWritten != 0);
			}

			static bool __Seek(FileInfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos)
			{
				NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

				s32 whence = NAND_SEEK_CUR;

				if (mode == __SEEK_ORIGIN)
					whence = NAND_SEEK_SET;
				else if (mode == __SEEK_END)
					whence = NAND_SEEK_END;
				else
					whence = NAND_SEEK_CUR;

				s32 result = NANDSeek(fileInfo, (s32)pos, whence);
				__private::__CheckNANDError(result);

				if (result >= 0)
					newPos = result;
				else
					newPos = pos;

				return (result >= 0);
			}
		};

		//------------------------------------------------------------------------------------------
		//---------------------------------- DVD ---------------------------------------------------
		//------------------------------------------------------------------------------------------
		namespace xwii_dvd
		{
			static bool __Init()
			{
				// DVDInit is called in xsystem_wii
				return true;
			}

			static bool __OpenOrCreateFile(FileInfo* pInfo)
			{
				DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
				pInfo->m_nFileHandle = (u32)dvdFileInfo;
				xbool boSuccess = DVDOpen(pInfo->m_szFilename, dvdFileInfo);
				return boSuccess;
			}

			static bool __LengthOfFile(FileInfo* pInfo, u64& outLength)
			{
				DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
				u64 l = (u64)DVDGetLength(dvdFileInfo);
				l = (l + 31) & 0xffffffe0;											///< DVDRead requires size to be a multiple of 32 so we better return a file size that fulfills this requirement
				outLength = l;
				return true;
			}

			static bool __CloseFile(FileInfo* pInfo)
			{
				DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
				xbool boSuccess = DVDClose(dvdFileInfo);
				pInfo->m_nFileHandle = INVALID_FILE_HANDLE;
				return boSuccess;
			}

			static bool __DeleteFile(FileInfo* pInfo)
			{
				return false;
			}

			static bool __ReadFile(FileInfo* pInfo, void* buffer, s32 size, u64 offset, u64& outNumBytesRead)
			{
				DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
				size = (size + 31) & 0xffffffe0;									///< DVDRead requires size to be a multiple of 32
				s32 numBytesRead = DVDRead(dvdFileInfo, buffer, size, offset);
				bool boSuccess = numBytesRead >= 0;
				if (boSuccess)
				{
					outNumBytesRead = numBytesRead;
				}
				return boSuccess;
			}

			static bool __WriteFile(FileInfo* pInfo, const void* buffer, s32 count, u64& outNumBytesWritten)
			{
				return false;
			}

			static bool __Seek(FileInfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos)
			{
				DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
				if (mode == __SEEK_END)
					pos = (u64)DVDGetLength(dvdFileInfo) - pos;
				pos = (pos + 3) & 0xfffffffc;
				s32 nResult = DVDSeek(dvdFileInfo, (s32)pos);
				if (nResult==0)
					newPos = pos;
				return nResult==0;
			}
		};

		namespace xwii_cstdio
		{
			bool __OpenOrCreateFile(FileInfo* pInfo)
			{
				char buffer[FS_MAX_PATH];

				xstring_buffer filename(buffer, sizeof(buffer));

				filename = pInfo->m_szFilename;
				
				FindAndRemoveAliasFromFilename(filename);

				std::FILE * pfile( std::fopen(buffer, "wb") );

				if(pfile)
				{
					// std::setbuf(pfile, 0);
					*( reinterpret_cast<std::FILE * *>(pInfo) ) = pfile;
					return true;
				}
				else
				{
					return false;
				}
			}

			bool __Seek(FileInfo* p, EWiiSeekMode, u64, u64&) // dummy
			{
				return p ? true : false;
			}

			bool __WriteFile(FileInfo* pInfo, const void* buffer, s32 count, u64& outNumBytesWritten)
			{
				outNumBytesWritten = std::fwrite(buffer, 1, count, *( reinterpret_cast<std::FILE * *>(pInfo) ) );
					
				return outNumBytesWritten > 0;
			}
			bool __Flush(FileInfo* pInfo)
			{
				return std::fflush( *( reinterpret_cast<std::FILE * *>(pInfo) ) ) == 0 ? true : false;
			}
			bool __CloseFile(FileInfo* pInfo)
			{
				pInfo->m_nFileHandle = INVALID_FILE_HANDLE; 
				return std::fclose(*( reinterpret_cast<std::FILE * *>(pInfo) ) ) ? true : false;
			}
		}

		//------------------------------------------------------------------------------------------
		//---------------------------------- Interface ---------------------------------------------
		//------------------------------------------------------------------------------------------

		static bool __OpenOrCreateFile(FileInfo* pInfo)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_HOST:	return xwii_hio2::__OpenOrCreateFile(pInfo);
			case FS_SOURCE_HDD:		return xwii_nand::__OpenOrCreateFile(pInfo);
			case FS_SOURCE_CSTDIO:  return xwii_cstdio::__OpenOrCreateFile(pInfo);
			case FS_SOURCE_DVD:		
			default:				return xwii_dvd::__OpenOrCreateFile(pInfo);
			}
		}
		static bool __LengthOfFile(FileInfo* pInfo, u64& outLength)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_HOST:	return xwii_hio2::__LengthOfFile(pInfo, outLength);
			case FS_SOURCE_HDD:		return xwii_nand::__LengthOfFile(pInfo, outLength);
			case FS_SOURCE_DVD:		
			default:				return xwii_dvd::__LengthOfFile(pInfo, outLength);
			}
		}
		static bool __Flush(FileInfo* pInfo)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_CSTDIO:  return xwii_cstdio::__Flush(pInfo);
			default:
				return true;
			}		
		}
		static bool __CloseFile(FileInfo* pInfo)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_HOST:	return xwii_hio2::__CloseFile(pInfo);
			case FS_SOURCE_HDD:		return xwii_nand::__CloseFile(pInfo);
			case FS_SOURCE_CSTDIO:  return xwii_cstdio::__CloseFile(pInfo);
			case FS_SOURCE_DVD:		
			default:				return xwii_dvd::__CloseFile(pInfo);
			}		
		}
		static bool __DeleteFile(FileInfo* pInfo)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_HOST:	return xwii_hio2::__DeleteFile(pInfo);
			case FS_SOURCE_HDD:		return xwii_nand::__DeleteFile(pInfo);
			case FS_SOURCE_DVD:		
			default:				return xwii_dvd::__DeleteFile(pInfo);
			}
		}
		static bool __ReadFile(FileInfo* pInfo, void* buffer, s32 size, u64 offset, u64& outNumBytesRead)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_HOST:	return xwii_hio2::__ReadFile(pInfo, buffer, size, outNumBytesRead);
			case FS_SOURCE_HDD:		return xwii_nand::__ReadFile(pInfo, buffer, size, outNumBytesRead);
			case FS_SOURCE_DVD:		
			default:				return xwii_dvd::__ReadFile(pInfo, buffer, size, offset, outNumBytesRead);
			}
		}
		static bool __WriteFile(FileInfo* pInfo, const void* buffer, s32 count, u64& outNumBytesWritten)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_HOST:	return xwii_hio2::__WriteFile(pInfo, buffer, count, outNumBytesWritten);
			case FS_SOURCE_HDD:		return xwii_nand::__WriteFile(pInfo, buffer, count, outNumBytesWritten);
			case FS_SOURCE_CSTDIO:  return xwii_cstdio::__WriteFile(pInfo, buffer, count, outNumBytesWritten);
			case FS_SOURCE_DVD:		
			default:				return xwii_dvd::__WriteFile(pInfo, buffer, count, outNumBytesWritten);
			}
		}
		static bool __Seek(FileInfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos)
		{
			switch (pInfo->m_eSource)
			{
			case FS_SOURCE_HOST:	return xwii_hio2::__Seek(pInfo, mode, pos, newPos);
			case FS_SOURCE_HDD:		return xwii_nand::__Seek(pInfo, mode, pos, newPos);
			case FS_SOURCE_CSTDIO:  return xwii_cstdio::__Seek(pInfo, mode, pos, newPos);
			case FS_SOURCE_DVD:		
			default:				return xwii_dvd::__Seek(pInfo, mode, pos, newPos);
			}
		}
	};

	namespace xfilesystem
	{
		using namespace __private;

		//--------
		// Defines
		//--------
		#define FS_ASYNC_WORKER_THREAD_STACK_SIZE				(0x4000) // 16 kb
		#define FS_ASYNC_WORKER_THREAD_EXIT_CODE				(0xbee)
		#define FS_ASYNC_WORKER_THREAD_PRIO						(0)

	
		//---------
		// Forward declares
		//---------

		static OSThread					m_AsyncIOThread;
		static xbyte					m_pAsyncIOThreadStack[FS_ASYNC_WORKER_THREAD_STACK_SIZE];

		static FileInfo					m_OpenAsyncFile[FS_MAX_OPENED_FILES];

		static QueueItem				m_aAsyncQueue[FS_MAX_ASYNC_QUEUE_ITEMS];
		static xmtllist<QueueItem>		m_pAsyncQueueList[FS_PRIORITY_COUNT];
		static xmtllist<QueueItem>		m_pFreeQueueItemList;

		static AsyncIOInfo				m_AsyncIOData[FS_MAX_ASYNC_IO_OPS];
		static xmtllist<AsyncIOInfo>	m_pFreeAsyncIOList;
		static xmtllist<AsyncIOInfo>	m_pAsyncIOList;

		static u32						m_uFileListLength = 0;
		static char**					m_pszFileListData = NULL;


		//----------------
		// Private Methods
		//----------------

		void*	AsyncIOWorkerThread(void*)
		{
			while (xTRUE)
			{
				AsyncIOInfo* pAsync = AsyncIORemoveHead();
				if(pAsync)
				{
					if(pAsync->m_nFileIndex >=  0)
					{
						FileInfo* pxInfo = GetFileInfo(pAsync->m_nFileIndex);

						if(pAsync->m_nStatus == FILE_OP_STATUS_OPEN_PENDING)
						{
							pAsync->m_nStatus = FILE_OP_STATUS_OPENING;

							bool boError   = false;
							bool boSuccess = __OpenOrCreateFile(pxInfo);
							if (!boSuccess)
							{
								x_printf ("__OpenOrCreateFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
								boError = true;
							}
							else
							{
								u64 uSize; 
								boSuccess = __LengthOfFile(pxInfo, uSize);
								if (!boSuccess)
								{
									x_printf ("__LengthOfFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
									boError = true;
								}
								else
								{
									u64	uSectorSize = 4096;
									{
										u64 uPad = uSize % uSectorSize;
										if (uPad != 0)
										{
											uPad = uSectorSize - uPad;
										}

										u32 uRoundedSize			= (u32)(uSize + uPad);
										u32 uNumSectors 			= (u32)(uRoundedSize / uSectorSize);

										pxInfo->m_uByteOffset		= 0;
										pxInfo->m_uByteSize			= uSize;
										pxInfo->m_uSectorOffset		= 0;
										pxInfo->m_uSectorSize		= 0;		///< uSectorSize
										pxInfo->m_uNumSectors		= uNumSectors;
									}
								}
							}

							if (boError)
							{
								if (pxInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
								{
									bool boClose = __CloseFile(pxInfo);
									if (!boClose)
									{
										x_printf ("__CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
									}
									pxInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
								}
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_CLOSE_PENDING)
						{
							pAsync->m_nStatus	= FILE_OP_STATUS_CLOSING;

							bool boClose = __CloseFile(pxInfo);
							if (!boClose)
							{
								x_printf ("__CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}
							
							pxInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_DELETE_PENDING)
						{
							pAsync->m_nStatus	= FILE_OP_STATUS_DELETING;

							bool boClose = __CloseFile(pxInfo);
							if (!boClose)
							{
								x_printf ("__CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}
							else
							{
								bool boDelete = __DeleteFile(pxInfo);
								if (!boDelete)
								{
									x_printf ("__DeleteFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
								}
							}

							pxInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_STAT_PENDING)
						{
							pAsync->m_nStatus	= FILE_OP_STATUS_STATING;

							//@TODO: use stats

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_READ_PENDING)
						{
							u64	nPos;
							bool boSeek = __Seek(pxInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
							if (!boSeek)
							{
								x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_READING;

							u64 nReadSize;
							bool boRead = __ReadFile(pxInfo, pAsync->m_pReadAddress, (u32)pAsync->m_uReadWriteSize, pAsync->m_uReadWriteOffset, nReadSize);
							if (!boRead)
							{
								x_printf ("__ReadFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_WRITE_PENDING)
						{
							u64	nPos;
							bool boSeek = __Seek(pxInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
							if (!boSeek)
							{
								x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_WRITING;

							u64 nWriteSize;
							bool boWrite = __WriteFile(pxInfo, pAsync->m_pWriteAddress, (u32)pAsync->m_uReadWriteSize, nWriteSize);
							if (!boWrite)
							{
								x_printf ("__WriteFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
					}
					FreeAsyncIOAddToTail(pAsync);
				}
				else
				{
					OSSuspendThread(&m_AsyncIOThread);
				}
			}
		}

		//------------------------------------------------------------------------------------------

		void				AsyncIOWorkerResume()
		{
			if (OSIsThreadSuspended(&m_AsyncIOThread))
				OSResumeThread(&m_AsyncIOThread);
		}

		//------------------------------------------------------------------------------------------

		void				AsyncIOWorkerAllow()
		{
			OSSleepMicroseconds(1);
		}

		//------------------------------------------------------------------------------------------

		s32					AsyncIONumActiveSlots(const u32 uPriority)
		{
			ASSERT(uPriority<FS_PRIORITY_COUNT);
			return m_pAsyncQueueList[uPriority].getLength();
		}

		//------------------------------------------------------------------------------------------

		s32					AsyncIONumFreeSlots()
		{
			return m_pFreeQueueItemList.getLength();
		}

		//------------------------------------------------------------------------------------------

		AsyncIOInfo*		GetAsyncIOData		( u32 nSlot )
		{
			AsyncIOInfo* asyncIOInfo = &m_AsyncIOData[nSlot];
			return asyncIOInfo;
		}

		//------------------------------------------------------------------------------------------

		AsyncIOInfo*		FreeAsyncIOPop		( void )
		{
			AsyncIOInfo* asyncIOInfo = m_pFreeAsyncIOList.removeHead();
			return asyncIOInfo;
		}

		//------------------------------------------------------------------------------------------

		void				FreeAsyncIOAddToTail( AsyncIOInfo* asyncIOInfo )
		{
			m_pFreeAsyncIOList.addToTail(asyncIOInfo);
		}

		//------------------------------------------------------------------------------------------

		void				AsyncIOAddToTail	( AsyncIOInfo* asyncIOInfo )
		{
			m_pAsyncIOList.addToTail(asyncIOInfo);
		}

		//------------------------------------------------------------------------------------------

		QueueItem*			FreeAsyncQueuePop	( )
		{
			QueueItem* item = m_pFreeQueueItemList.removeHead();
			return item;
		}

		//------------------------------------------------------------------------------------------
		
		void				FreeAsyncQueueAddToTail( QueueItem* asyncQueueItem )
		{
			m_pFreeQueueItemList.addToTail(asyncQueueItem);
		}

		//------------------------------------------------------------------------------------------

		QueueItem*			AsyncQueueRemoveHead( u32 uPriority )
		{
			QueueItem* item = m_pAsyncQueueList[uPriority].removeHead();
			return item;
		}

		//------------------------------------------------------------------------------------------

		void				AsyncQueueAddToTail	( u32 uPriority, QueueItem* asyncQueueItem )
		{
			m_pAsyncQueueList[uPriority].addToTail(asyncQueueItem);
		}

		//------------------------------------------------------------------------------------------

		void				AsyncQueueAddToHead	( u32 uPriority, QueueItem* asyncQueueItem )
		{
			m_pAsyncQueueList[uPriority].addToHead(asyncQueueItem);
		}

		//------------------------------------------------------------------------------------------

		FileInfo*			GetFileInfo			( u32 uHandle )
		{
			ASSERT(uHandle<FS_MAX_OPENED_FILES);
			return &m_OpenAsyncFile[uHandle];
		}

		//------------------------------------------------------------------------------------------

		u32					FindFreeFileSlot (void)
		{
			for (s32 nSlot = 0; nSlot < FS_MAX_OPENED_FILES; nSlot++)
			{
				if (m_OpenAsyncFile[nSlot].m_nFileHandle == (u32)INVALID_FILE_HANDLE)
					return (nSlot);
			}
			return (u32)-1;
		}

		//---------------
		// Public Methods
		//---------------

		void				Initialise ( u32 uAsyncQueueSize, xbool boEnableCache )
		{
			InitialiseCommon(uAsyncQueueSize, boEnableCache);

			//--------------------------------
			// Create the Async loading queue.
			//--------------------------------
			ASSERTS (uAsyncQueueSize > 0 && uAsyncQueueSize <= FS_MAX_ASYNC_QUEUE_ITEMS, "Initialise() : Async Queue is 0 or to large!");

			//	m_nAsyncQueueHead		= 0;
			//	m_nAsyncQueueTail		= 0;

			xwii_dvd::__Init();
			xwii_hio2::__Init();

			//--------------------------------------------------
			// Fill in the status of the first element in queue.
			//--------------------------------------------------
			for (u32 uLoop = 0; uLoop < FS_MAX_ASYNC_QUEUE_ITEMS; uLoop++)
			{
				m_aAsyncQueue[uLoop].clear();
				m_pFreeQueueItemList.addToTail(&(m_aAsyncQueue[uLoop]));
			}

			//---------------------------------------
			// Current there is no Async file loaded.
			//---------------------------------------
			for (u32 uFile = 0; uFile < FS_MAX_OPENED_FILES; uFile++)
			{
				m_OpenAsyncFile[uFile].clear();
			}

			for(u32 uSlot = 0; uSlot < FS_MAX_ASYNC_IO_OPS; uSlot++)	
			{
				m_AsyncIOData[uSlot].clear();
				m_pFreeAsyncIOList.addToTail(&m_AsyncIOData[uSlot]);
			}

			if (!OSCreateThread(&m_AsyncIOThread, AsyncIOWorkerThread, (void*)NULL, (void*)(m_pAsyncIOThreadStack + FS_ASYNC_WORKER_THREAD_STACK_SIZE), FS_ASYNC_WORKER_THREAD_STACK_SIZE, (OSPriority)6, 0))
			{
				x_printf ("Stdio:"TARGET_PLATFORM_STR" ERROR OSCreateThread failed\n");
			}
			OSSetThreadPriority (&m_AsyncIOThread, 6);

			if (boEnableCache)
				CreateFileCache();
		}	

		//------------------------------------------------------------------------------------------

		void				Shutdown ( void )
		{

			DestroyFileCache();

			OSDetachThread(&m_AsyncIOThread);
			OSCancelThread(&m_AsyncIOThread);

			x_printf ("Stdio:"TARGET_PLATFORM_STR" INFO Shutdown()\n");
		}


		//------------------------------------------------------------------------------------------

		void				GetOpenCreatedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenCreatedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		void				GetOpenModifiedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		void				ReSize( u32 uHandle, u64 uNewSize )
		{
			FileInfo* pInfo = &m_OpenAsyncFile[uHandle];

			s32 nResult=-1;
// 			nResult	= cellFsFtruncate(pInfo->m_nFileHandle, uNewSize);
			if(nResult < 0)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR ReSize %d\n", x_va_list(nResult));
			}
		}

		//------------------------------------------------------------------------------------------

		u64					GetFreeSize( const char* szPath )
		{
			u32	uBlockSize  = 0;
			u64	uBlockCount = 0;
			//cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

			return uBlockSize * uBlockCount;
		}

		//------------------------------------------------------------------------------------------
		void				__ParseDirDvd(xstring_buffer& szDir, bool boRecursive, u32& ruFileList, char** pszFileList)
		{
			//@TODO: This could easily kill the stack on the WII, we only have 64 Kilobyte

			// Start the directory read
			DVDDir dvdDir;
			DVDOpenDir(szDir.c_str(), &dvdDir);

			while (xTRUE)
			{
				DVDDirEntry dirent;
				bool uRead = DVDReadDir(&dvdDir, &dirent);

				if (uRead)
				{
					if (dirent.isDir)
					{
						if(	(boRecursive == true) && (x_stricmp(dirent.name, ".") != 0) && (x_stricmp(dirent.name, "..") != 0) )
						{
							const s32 start = szDir.getLength();
							const s32 len = x_strlen(dirent.name);
							szDir += dirent.name;
							ParseDir(szDir, boRecursive, ruFileList, pszFileList);
							szDir.remove(start, len);
						}
					}
					else
					{
						if (pszFileList)
						{
							// Allocate enough memory for the new entry, and copy it over
							// pszFileList[ruFileList]	= (char*)HeapManager::GetHeap()->AllocFromEnd(x_strlen(szDir) + xDirInfo.d_namlen + 1);
							s32 maxLen = x_strlen(szDir) + x_strlen(dirent.name) + 1;
							pszFileList[ruFileList]	= (char*)x_malloc(sizeof(xbyte), maxLen, XMEM_FLAG_ALIGN_8B);
							x_strcpy(pszFileList[ruFileList], maxLen, szDir);
							x_strcat(pszFileList[ruFileList], maxLen, dirent.name);
						}
						ruFileList++;
					}
				}
				else
				{
					break;
				}
			}

			// Done
			DVDCloseDir(&dvdDir);
		}

		//------------------------------------------------------------------------------------------

		void				CreateFileList( const char* szPath, xbool boRecursive )
		{
			ASSERTS(m_uFileListLength == 0, "CreateFileList Already exists");
			ASSERTS(m_pszFileListData == NULL, "CreateFileList Already exists - memory leak occurring!\n");

			m_uFileListLength	= 0;
			m_pszFileListData	= NULL;

			// Parse the dir twice - once to see memory usage, 2nd to get data
			ParseDir(szPath, boRecursive, m_uFileListLength, m_pszFileListData);

			if(m_uFileListLength > 0)
			{
				// Allocate and fill info
				// m_pszFileListData	= (char**)HeapManager::GetHeap()->AllocFromEnd(m_uFileListLength * sizeof(char*));
				m_pszFileListData = (char**)x_malloc(sizeof(char*), m_uFileListLength, XMEM_FLAG_ALIGN_8B);

				u32	uIndex	= 0;
				ParseDir(szPath, boRecursive, uIndex, m_pszFileListData);
			}
		}

		//------------------------------------------------------------------------------------------

		void			DestroyFileList( void )
		{
			// Done - free all buffers
			for(u32 uFile = 0; uFile < m_uFileListLength; uFile++)
			{
				x_free(m_pszFileListData[uFile]);
			}

			x_free(m_pszFileListData);

			m_uFileListLength	= 0;
			m_pszFileListData	= NULL;
		}

		//------------------------------------------------------------------------------------------

		s32				GetFileListLength	( void )
		{
			return m_uFileListLength;
		}

		//------------------------------------------------------------------------------------------

		const char*		GetFileListData		( u32 nFile )
		{
			if (nFile < m_uFileListLength && m_pszFileListData != NULL)
				return m_pszFileListData[nFile];

			return "";
		}

		//////////////////////////////////////////////////////////////////////////
		// Private xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////
		namespace __private
		{
			//------------------------------------------------------------------------------------------

			void __ParseDirDvd(xstring_buffer& szDir, bool boRecursive, u32& ruFileList, char** pszFileList)
			{
				//@TODO: This could easily kill the stack on the WII, we only have 64 Kilobyte

				// Start the directory read
				DVDDir dvdDir;
				DVDOpenDir(szDir.c_str(), &dvdDir);

				while (xTRUE)
				{
					DVDDirEntry dirent;
					bool uRead = DVDReadDir(&dvdDir, &dirent);

					if (uRead)
					{
						if (dirent.isDir)
						{
							if(	(boRecursive == true) && (x_stricmp(dirent.name, ".") != 0) && (x_stricmp(dirent.name, "..") != 0) )
							{
								const s32 start = szDir.getLength();
								const s32 len = x_strlen(dirent.name);
								szDir += dirent.name;
								ParseDir(szDir, boRecursive, ruFileList, pszFileList);
								szDir.remove(start, len);
							}
						}
						else
						{
							if (pszFileList)
							{
								// Allocate enough memory for the new entry, and copy it over
								// pszFileList[ruFileList]	= (char*)HeapManager::GetHeap()->AllocFromEnd(x_strlen(szDir) + xDirInfo.d_namlen + 1);
								s32 maxLen = x_strlen(szDir) + x_strlen(dirent.name) + 1;
								pszFileList[ruFileList]	= (char*)x_malloc(sizeof(xbyte), maxLen, XMEM_FLAG_ALIGN_8B);
								x_strcpy(pszFileList[ruFileList], maxLen, szDir);
								x_strcat(pszFileList[ruFileList], maxLen, dirent.name);
							}
							ruFileList++;
						}
					}
					else
					{
						break;
					}
				}

				// Done
				DVDCloseDir(&dvdDir);
			}

			void ParseDir(const char* szDir, xbool boRecursive, u32& ruFileList, char** pszFileList)
			{
				XSTRING_BUFFER(szFullPath, FS_MAX_PATH);
				ESourceType eSource = CreateSystemPath(szDir, szFullPath);

				if (eSource == FS_SOURCE_DVD)
				{
					__ParseDirDvd(szFullPath, boRecursive, ruFileList, pszFileList);
				}
			}

			//------------------------------------------------------------------------------------------

			xbool			IsPathUNIXStyle		( void )
			{
				return true;
			}


			//------------------------------------------------------------------------------------------

			void				AsyncIOWorkerResume()
			{
				if (OSIsThreadSuspended(&m_AsyncIOThread))
					OSResumeThread(&m_AsyncIOThread);
			}

			//------------------------------------------------------------------------------------------
			///< Synchronous file operations

			u32					SyncOpen			( const char* szName, xbool boWrite, xbool boRetry)
			{
				u32 uHandle = AsyncPreOpen(szName, boWrite);
				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__OpenOrCreateFile(pxFileInfo))
				{
					x_printf ("__OpenOrCreateFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
					pxFileInfo->clear();
					uHandle = (u32)INVALID_FILE_HANDLE;
				}
				return uHandle;
			}

			//------------------------------------------------------------------------------------------

			uintfs				SyncSize			( u32 uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return 0;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				u64 length;
				if (!__LengthOfFile(pxFileInfo, length))
				{
					x_printf ("__LengthOfFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				return length;
			}

			//------------------------------------------------------------------------------------------

			void				SyncRead			( u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry)
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				u64 numBytesRead;
				if (!__ReadFile(pxFileInfo, pBuffer, uSize, uOffset, numBytesRead))
				{
					x_printf ("__ReadFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
			}

			//------------------------------------------------------------------------------------------

			void				SyncWrite			( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry)
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				u64 newOffset;
				if (!__Seek(pxFileInfo, __SEEK_ORIGIN, uOffset, newOffset))
				{
					x_printf ("__Seek failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				u64 numBytesWritten;
				if (!__WriteFile(pxFileInfo, pBuffer, uSize, numBytesWritten))
				{
					x_printf ("__WriteFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
			}

			void				SyncFlush			( u32 uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);

				if (!__Flush(pxFileInfo))
				{
					x_printf ("__Flush failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
			}

			//------------------------------------------------------------------------------------------

			void 				SyncClose			( u32& uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__CloseFile(pxFileInfo))
				{
					x_printf ("__CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}

			//------------------------------------------------------------------------------------------

			void				SyncDelete			( u32& uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__CloseFile(pxFileInfo))
				{
					x_printf ("__CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				__DeleteFile(pxFileInfo);
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}
		};
	};


	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_WII