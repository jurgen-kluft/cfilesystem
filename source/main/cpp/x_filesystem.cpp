//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_threading.h"
#include "xfilesystem/x_filedevice.h"

#include "xfilesystem/private/x_fileasync.h"
#include "xfilesystem/private/x_filedata.h"
#include "xfilesystem/private/x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		void doIO(xio_thread* io_thread)
		{
			const xcore::s32 queueSize = 5;
			xfileasync* pAsync = NULL;
			xcore::s32 loopFlag = queueSize; // when the thread is stopped loop extra times to try to clean out memory in queues
 			
			io_thread->wait(); // wait before starting loop
			
			do
			{
				// parameters sent out by the callback
				x_asyncio_result ioResult;
				ioResult.userData = NULL;
				ioResult.result = 0;
				ioResult.buffer = NULL;
				ioResult.fileHandle = (u32)INVALID_FILE_HANDLE;
				ioResult.operation = FILE_OP_STATUS_FREE;

				pAsync = xfs_common::s_instance()->popAsyncIO();
				if (pAsync)
				{
					if (pAsync->getFileIndex() >=  0)
					{
						xfiledata* pInfo = xfs_common::s_instance()->getFileInfo(pAsync->getFileIndex());
						xfiledevice* pFileDevice = pInfo->m_pFileDevice;

						// ========================================================================

						if (pFileDevice!=NULL && pInfo!=NULL)
						{

							ioResult.userData = pAsync->getCallback().userData; // Set the user data void ptr

							if (pAsync->getStatus() == FILE_OP_STATUS_OPEN_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_OPENING);

								bool boError   = false;
								u32 nFileHandle;
								bool boSuccess = pFileDevice->openFile(pInfo->m_szFilename, pInfo->m_boReading, pInfo->m_boWriting, nFileHandle);
								if (!boSuccess)
								{
									x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR openFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
									boError = true;
								}
								else
								{
									pInfo->m_nFileHandle = nFileHandle;
									pFileDevice->getLengthOfFile(nFileHandle, pInfo->m_uByteSize);
								}

								if (boError)
								{
									if (pInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
									{
										bool boClose = pFileDevice->closeFile(pInfo->m_nFileHandle);
										if (!boClose)
										{
											x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR closeFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
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
									x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR closeFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
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
									x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR closeFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
								}
								else
								{
									bool boDelete = pFileDevice->deleteFile(pInfo->m_szFilename);
									if (!boDelete)
									{
										x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR deleteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
									}
								}

								pInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							
							else if (pAsync->getStatus() == FILE_OP_STATUS_READ_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_READING);

								u64 readOffset = xfs_common::s_instance()->getpos(pAsync->getFileIndex());
								bool boRead = pFileDevice->readFile(pInfo->m_nFileHandle, readOffset, pAsync->getReadAddress(), (u32)pAsync->getReadWriteSize(), ioResult.result);
								if (!boRead)
								{
									x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR readFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
									
									ioResult.buffer = NULL;
									ioResult.operation = FILE_OP_STATUS_READ_ERROR;
								
								}

								else
								{

									ioResult.buffer = (xcore::xbyte*)pAsync->getReadAddress();
									ioResult.operation = FILE_OP_STATUS_READ_FINISHED;
								}

								ioResult.fileHandle = pInfo->m_nFileHandle;


								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							else if (pAsync->getStatus() == FILE_OP_STATUS_WRITE_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_WRITING);


								// NOTICE: we can not use pAsync->getReadWriteOffset() since this was generated in the wrong thread while we may be writing to the same file here.
								// instead, compute base write offset right now
								u64 writeOffset = xfs_common::s_instance()->getpos(pAsync->getFileIndex());
								bool boWrite = pFileDevice->writeFile(pInfo->m_nFileHandle, writeOffset, pAsync->getWriteAddress(), (u32)pAsync->getReadWriteSize(), ioResult.result);
								if (!boWrite)
								{
									x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR writeFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
									ioResult.buffer = NULL;
									ioResult.operation = FILE_OP_STATUS_WRITE_ERROR;
								
								}

								else
								{

									ioResult.buffer = (xcore::xbyte*)pAsync->getWriteAddress();
									ioResult.operation = FILE_OP_STATUS_WRITE_FINISHED;
								}

								ioResult.fileHandle = pInfo->m_nFileHandle;

								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
						}
						// ========================================================================


						// Operation is finished, call the callback
						if (pAsync->getStatus() == FILE_OP_STATUS_DONE)
						{
							x_asyncio_callback_struct callback = pAsync->getCallback();

							callback.callback(ioResult);

							xfilesystem::xfs_common::s_instance()->pushFreeFileSlot(pAsync->getFileIndex());

							io_thread->wait(); // wait only after successful IO
							
						}


						pAsync->clear();
						xfs_common::s_instance()->pushFreeAsyncIO(pAsync);
					}
				}


				if(!io_thread->loop())
				{
					loopFlag--;
				}
			} while (loopFlag >= 0 || pAsync != NULL);



		}
		
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
