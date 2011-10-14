#ifndef __X_FILESYSTEM_FILEASYNC_H__
#define __X_FILESYSTEM_FILEASYNC_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"

#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		// Forward declares
		class xevent;

		enum EFileOpStatus
		{
			FILE_OP_STATUS_FREE				= 0,
			FILE_OP_STATUS_DONE				= 0,

			FILE_OP_STATUS_OPEN_PENDING		= 10,
			FILE_OP_STATUS_OPENING			= 11,

			FILE_OP_STATUS_CLOSE_PENDING	= 110,
			FILE_OP_STATUS_CLOSING			= 111,

			FILE_OP_STATUS_READ_PENDING		= 210,
			FILE_OP_STATUS_READING			= 211,

			FILE_OP_STATUS_WRITE_PENDING	= 310,
			FILE_OP_STATUS_WRITING			= 311,

			FILE_OP_STATUS_DELETE_PENDING	= 410,
			FILE_OP_STATUS_DELETING			= 411,
		};

		struct xfileasync
		{
			XFILESYSTEM_OBJECT_NEW_DELETE()

			s32					getFileIndex() const							{ return m_nFileIndex; }
			EFileOpStatus		getStatus() const								{ return (EFileOpStatus)m_nStatus; }
			const void*			getWriteAddress() const							{ return m_pWriteAddress; }
			void*				getReadAddress() const							{ return m_pReadAddress; }
			u64					getReadWriteOffset() const						{ return m_uReadWriteOffset; }
			u64					getReadWriteSize() const						{ return m_uReadWriteSize; }

			bool				getPushFileDataOnFreeQueue() const				{ return m_nFileDataCmd == 1; }

			xevent*				getEvent()										{ return m_pEvent; }
			x_asyncio_callback_struct*		getCallback()									{ return m_fpCallback; }

			void				setFileIndex(s32 sIndex)						{ m_nFileIndex = sIndex; }
			void				setStatus(EFileOpStatus eStatus)				{ m_nStatus = eStatus; }
			void				setWriteAddress(const void* sAddress) 			{ m_pWriteAddress = sAddress; }
			void				setReadAddress(void* sAddress)					{ m_pReadAddress = sAddress; }
			void				setReadWriteOffset(u64 uOffset) 				{ m_uReadWriteOffset = uOffset; }
			void				setReadWriteSize(u64 uSize) 					{ m_uReadWriteSize = uSize; }

			void				setPushFileDataOnFreeQueue(bool flag)			{ if (flag) m_nFileDataCmd |= 1; else m_nFileDataCmd &= 1; }

			void				setEvent(xevent* async_event)					{ m_pEvent = async_event; }
			void				setCallback(x_asyncio_callback_struct* callback)				{ m_fpCallback = callback; }

			void				clear()
			{
				m_nFileIndex	= -1;
				m_nFileDataCmd	= 0;
				m_nStatus		= FILE_OP_STATUS_FREE;
					
				m_pWriteAddress	= NULL;
				m_pReadAddress	= NULL;

				m_uReadWriteOffset	= 0;
				m_uReadWriteSize	= 0;

				m_pEvent		= NULL;
			}

		private:

			s32					m_nFileIndex;
			s32					m_nFileDataCmd;
			s32					m_nStatus;
			const void*			m_pWriteAddress;
			void*				m_pReadAddress;

			u64					m_uReadWriteOffset;
			u64					m_uReadWriteSize;

			xevent*				m_pEvent;
			x_asyncio_callback_struct*		m_fpCallback;
		};

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_FILEASYNC_H__
//==============================================================================
#endif