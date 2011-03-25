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

			FILE_OP_STATUS_STAT_PENDING		= 410,
			FILE_OP_STATUS_STATING			= 411,

			FILE_OP_STATUS_DELETE_PENDING	= 510,
			FILE_OP_STATUS_DELETING			= 511,
		};

		struct xfileasync
		{
			XFILESYSTEM_OBJECT_NEW_DELETE()

			xfileasync*			getPrev	() const								{ return m_pPrev; }
			xfileasync*			getNext	() const								{ return m_pNext; }

			s32					getFileIndex() const							{ return m_nFileIndex; }
			EFileOpStatus		getStatus() const								{ return (EFileOpStatus)m_nStatus; }
			const void*			getWriteAddress() const							{ return m_pWriteAddress; }
			void*				getReadAddress() const							{ return m_pReadAddress; }
			u64					getReadWriteOffset() const						{ return m_uReadWriteOffset; }
			u64					getReadWriteSize() const						{ return m_uReadWriteSize; }

			void				setPrev	( xfileasync* pPrev )					{ m_pPrev = pPrev; }
			void				setNext	( xfileasync* pNext )					{ m_pNext = pNext; }

			void				setFileIndex(s32 sIndex)						{ m_nFileIndex = sIndex; }
			void				setStatus(EFileOpStatus eStatus)				{ m_nStatus = eStatus; }
			void				setWriteAddress(const void* sAddress) 			{ m_pWriteAddress = sAddress; }
			void				setReadAddress(void* sAddress)					{ m_pReadAddress = sAddress; }
			void				setReadWriteOffset(u64 uOffset) 				{ m_uReadWriteOffset = uOffset; }
			void				setReadWriteSize(u64 uSize) 					{ m_uReadWriteSize = uSize; }

			void				clear()
			{
				m_pPrev			= NULL;
				m_pNext			= NULL;

				m_nFileIndex	= -1;
				m_nStatus		= FILE_OP_STATUS_FREE;
					
				m_pWriteAddress	= NULL;
				m_pReadAddress	= NULL;

				m_uReadWriteOffset	= 0;
				m_uReadWriteSize	= 0;
			}

		private:
			xfileasync*			m_pPrev;
			xfileasync*			m_pNext;

			s32					m_nFileIndex;
			s32					m_nStatus;
			const void*			m_pWriteAddress;
			void*				m_pReadAddress;

			u64					m_uReadWriteOffset;
			u64					m_uReadWriteSize;
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