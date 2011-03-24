//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_threading.h"
#include "xfilesystem\x_filedevice.h"

#include "xfilesystem\private\x_fileasync.h"
#include "xfilesystem\private\x_fileinfo.h"
#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		void IoThreadWorker()
		{
			do
			{
				xfileasync* pAsync = asyncIORemoveHead();
				if (pAsync)
				{
					if (pAsync->getFileIndex() >=  0)
					{
						xfiledata* pInfo = getFileInfo(pAsync->getFileIndex());
						xfiledevice* pFileDevice = pInfo->m_pFileDevice;

						// ========================================================================

						if (pFileDevice!=NULL && pInfo!=NULL)
						{
							if (pAsync->getStatus() == FILE_OP_STATUS_OPEN_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_OPENING);

								bool boError   = false;
								u32 nFileHandle;
								bool boSuccess = pFileDevice->openOrCreateFile(pInfo->m_nFileIndex, pInfo->m_szFilename, pInfo->m_boReading, pInfo->m_boWriting, nFileHandle);
								if (!boSuccess)
								{
									x_printf ("openOrCreateFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
									boError = true;
								}
								else
								{
									pInfo->m_nFileHandle = nFileHandle;
									pFileDevice->lengthOfFile(nFileHandle, pInfo->m_uByteSize);
								}

								if (boError)
								{
									if (pInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
									{
										bool boClose = pFileDevice->closeFile(pInfo->m_nFileHandle);
										if (!boClose)
										{
											x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
										}
										pInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
									}
								}

								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							else if (pAsync->getStatus() == FILE_OP_STATUS_CLOSE_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_CLOSING);

								bool boClose = pFileDevice->closeFile(pInfo->m_nFileHandle);
								if (!boClose)
								{
									x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
								}
							
								pInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							else if (pAsync->getStatus() == FILE_OP_STATUS_DELETE_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_DELETING);

								bool boClose = pFileDevice->closeFile(pInfo->m_nFileHandle);
								if (!boClose)
								{
									x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
								}
								else
								{
									bool boDelete = pFileDevice->deleteFile(pInfo->m_nFileHandle, pInfo->m_szFilename);
									if (!boDelete)
									{
										x_printf ("DeleteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
									}
								}

								pInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							else if (pAsync->getStatus() == FILE_OP_STATUS_STAT_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_STATING);

								//@TODO: use stats

								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							else if (pAsync->getStatus() == FILE_OP_STATUS_READ_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_READING);

								u64 nReadSize;
								bool boRead = pFileDevice->readFile(pInfo->m_nFileHandle, pAsync->getReadWriteOffset(), pAsync->getReadAddress(), (u32)pAsync->getReadWriteSize(), nReadSize);
								if (!boRead)
								{
									x_printf ("ReadFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
								}

								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							else if (pAsync->getStatus() == FILE_OP_STATUS_WRITE_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_WRITING);

								u64 nWriteSize;
								bool boWrite = pFileDevice->writeFile(pInfo->m_nFileHandle, pAsync->getReadWriteOffset(), pAsync->getWriteAddress(), (u32)pAsync->getReadWriteSize(), nWriteSize);
								if (!boWrite)
								{
									x_printf ("WriteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
								}

								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
						}
						// ========================================================================

						freeAsyncIOAddToTail(pAsync);
					}
				}
				else
				{
					getThreading()->wait();
				}
			} while (getThreading()->loop());
		}
		
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
