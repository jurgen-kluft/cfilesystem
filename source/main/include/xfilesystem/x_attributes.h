#ifndef __X_FILESYSTEM_FILE_ATTRIBUTES_AND_TIMES_H__
#define __X_FILESYSTEM_FILE_ATTRIBUTES_AND_TIMES_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_types.h"
#include "xtime/x_time.h"

//==============================================================================
namespace xcore
{
	struct xfileattrs
	{
								xattributes();
								xattributes(const xattributes&);
								xattributes(bool boArchive, bool boReadonly, bool boHidden, bool boSystem);

		bool					isArchive() const;
		bool					isReadOnly() const;
		bool					isHidden() const;
		bool					isSystem() const;

		void					setArchive(bool);
		void					setReadOnly(bool);
		void					setHidden(bool);
		void					setSystem(bool);

		xattributes&			operator = (const xattributes&);

		bool					operator == (const xattributes&) const;
		bool					operator != (const xattributes&) const;

	private:
		xcore::u32				mFlags;
	};


	struct xfiletimes
	{
		bool					getTime(xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const;
		bool					getCreationTime  (xdatetime&) const;
		bool					getLastAccessTime(xdatetime&) const;
		bool					getLastWriteTime (xdatetime&) const;

		bool					setTime(const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime);
		bool					setCreationTime(const xdatetime&);
		bool					setLastAccessTime(const xdatetime&);
		bool					setLastWriteTime (const xdatetime&);

	private:
		xdatetime				m_creationtime;
		xdatetime				m_lastaccesstime;
		xdatetime				m_lastwritetime;
	};

};

#endif
