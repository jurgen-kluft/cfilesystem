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
#include "xfilesystem\private\x_filedata.h"
#include "xfilesystem\private\x_filesystem_common.h"

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
 			do
			{
				xcore::u64 nResultSize = 0;

				pAsync = popAsyncIO();
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

								bool boRead = pFileDevice->readFile(pInfo->m_nFileHandle, pAsync->getReadWriteOffset(), pAsync->getReadAddress(), (u32)pAsync->getReadWriteSize(), nResultSize);
								if (!boRead)
								{
									x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR readFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
								}

								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
							else if (pAsync->getStatus() == FILE_OP_STATUS_WRITE_PENDING)
							{
								pAsync->setStatus(FILE_OP_STATUS_WRITING);

								bool boWrite = pFileDevice->writeFile(pInfo->m_nFileHandle, pAsync->getReadWriteOffset(), pAsync->getWriteAddress(), (u32)pAsync->getReadWriteSize(), nResultSize);
								if (!boWrite)
								{
									x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR writeFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
								}

								pAsync->setStatus(FILE_OP_STATUS_DONE);
							}
						}
						// ========================================================================


						// Operation is finished, call the callback
						if (pAsync->getStatus() == FILE_OP_STATUS_DONE)
						{
							AsyncCallback callback = pAsync->getCallback();

							callback(nResultSize, (xcore::xbyte*)pAsync->getWriteAddress());

							xfilesystem::pushFreeFileSlot(pAsync->getFileIndex());
							
						}

						// no longer care about xevent
						/*
						if (pAsync->getEvent() != NULL)
						{
							pAsync->getEvent()->signal();
							getEventFactory()->destruct(pAsync->getEvent());
						}*/

						pAsync->clear();
						pushFreeAsyncIO(pAsync);
					}
				}
				else
				{
					io_thread->wait();
				}

				if(!io_thread->loop())
				{
					loopFlag--;
				}
			} while (loopFlag >= 0 || pAsync != NULL);



			// more attempt to clean the queue of event allocations
			for(xcore::s32 i = 0; i < queueSize; i++)
			{
				pAsync = popAsyncIO();

				if(pAsync != NULL)
				{
					if (pAsync->getEvent() != NULL)
					{
						pAsync->getEvent()->signal();
						getEventFactory()->destruct(pAsync->getEvent());
					}
				}
			}


		}
		
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
