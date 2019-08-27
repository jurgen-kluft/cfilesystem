#ifndef __X_FILESYSTEM_FILE_ATTRIBUTES_AND_TIMES_H__
#define __X_FILESYSTEM_FILE_ATTRIBUTES_AND_TIMES_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xtime/x_datetime.h"

namespace xcore
{
    class xfileattrs
    {
	public:
        xfileattrs();
        xfileattrs(const xfileattrs&);
        xfileattrs(bool boArchive, bool boReadonly, bool boHidden, bool boSystem);

        bool isArchive() const;
        bool isReadOnly() const;
        bool isHidden() const;
        bool isSystem() const;

        void setArchive(bool);
        void setReadOnly(bool);
        void setHidden(bool);
        void setSystem(bool);

        xfileattrs& operator=(const xfileattrs&);

        bool operator==(const xfileattrs&) const;
        bool operator!=(const xfileattrs&) const;

    private:
        xcore::u32 mFlags;
    };

    class xfiletimes
    {
	public:
        xfiletimes();
        xfiletimes(const xfiletimes&);
        xfiletimes(const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime);

        void getTime(xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const;
        void getCreationTime(xdatetime&) const;
        void getLastAccessTime(xdatetime&) const;
        void getLastWriteTime(xdatetime&) const;

        void setTime(const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime);
        void setCreationTime(const xdatetime&);
        void setLastAccessTime(const xdatetime&);
        void setLastWriteTime(const xdatetime&);

        xfiletimes& operator=(const xfiletimes&);

        bool operator==(const xfiletimes&) const;
        bool operator!=(const xfiletimes&) const;

    private:
        xdatetime m_creationtime;
        xdatetime m_lastaccesstime;
        xdatetime m_lastwritetime;
    };
}; // namespace xcore

#endif
