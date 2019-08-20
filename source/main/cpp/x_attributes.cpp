//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_target.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_attributes.h"

namespace xcore
{
    enum EAttributes
    {
        FILE_ATTRIBUTE_ARCHIVE  = 1,
        FILE_ATTRIBUTE_READONLY = 2,
        FILE_ATTRIBUTE_HIDDEN   = 4,
        FILE_ATTRIBUTE_SYSTEM   = 8,
    };

    xfileattrs::xfileattrs()
        : mFlags(0)
    {
    }

    xfileattrs::xfileattrs(const xfileattrs& other)
        : mFlags(other.mFlags)
    {
    }

    xfileattrs::xfileattrs(bool boArchive, bool boReadonly, bool boHidden, bool boSystem)
        : mFlags(0)
    {
        setArchive(boArchive);
        setReadOnly(boReadonly);
        setHidden(boHidden);
        setSystem(boSystem);
    }

    bool xfileattrs::isArchive() const { return (mFlags & FILE_ATTRIBUTE_ARCHIVE) != 0; }

    bool xfileattrs::isReadOnly() const { return (mFlags & FILE_ATTRIBUTE_READONLY) != 0; }

    bool xfileattrs::isHidden() const { return (mFlags & FILE_ATTRIBUTE_HIDDEN) != 0; }

    bool xfileattrs::isSystem() const { return (mFlags & FILE_ATTRIBUTE_SYSTEM) != 0; }

    void xfileattrs::setArchive(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_ARCHIVE;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_ARCHIVE;
    }

    void xfileattrs::setReadOnly(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_READONLY;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_READONLY;
    }

    void xfileattrs::setHidden(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_HIDDEN;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_HIDDEN;
    }

    void xfileattrs::setSystem(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_SYSTEM;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_SYSTEM;
    }

    xfileattrs& xfileattrs::operator=(const xfileattrs& other)
    {
        mFlags = other.mFlags;
        return *this;
    }

    bool xfileattrs::operator==(const xfileattrs& other) const { return mFlags == other.mFlags; }

    bool xfileattrs::operator!=(const xfileattrs& other) const { return mFlags != other.mFlags; }

    xfiletimes::xfiletimes() {}
    xfiletimes::xfiletimes(const xfiletimes& ftimes)
        : m_creationtime(ftimes.m_creationtime)
        , m_lastaccesstime(ftimes.m_lastaccesstime)
        , m_lastwritetime(ftimes.m_lastwritetime)
    {
    }

    xfiletimes::xfiletimes(const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime)
        : m_creationtime(creationTime)
        , m_lastaccesstime(lastAccessTime)
        , m_lastwritetime(lastWriteTime)
    {
    }

    void xfiletimes::getTime(xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
    {
        outCreationTime   = m_creationtime;
        outLastAccessTime = m_lastaccesstime;
        outLastWriteTime  = m_lastwritetime;
    }

    void xfiletimes::getCreationTime(xdatetime& dt) const { dt = m_creationtime; }

    void xfiletimes::getLastAccessTime(xdatetime& dt) const { dt = m_lastaccesstime; }

    void xfiletimes::getLastWriteTime(xdatetime& dt) const { dt = m_lastwritetime; }

    void xfiletimes::setTime(const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime)
    {
        m_creationtime   = creationTime;
        m_lastaccesstime = lastAccessTime;
        m_lastwritetime  = lastWriteTime;
    }

    void xfiletimes::setCreationTime(const xdatetime& dt) { m_creationtime = dt; }

    void xfiletimes::setLastAccessTime(const xdatetime& dt) { m_lastaccesstime = dt; }

    void xfiletimes::setLastWriteTime(const xdatetime& dt) { m_lastwritetime = dt; }

    xfiletimes& xfiletimes::operator=(const xfiletimes& other)
    {
        m_creationtime   = other.m_creationtime;
        m_lastaccesstime = other.m_lastaccesstime;
        m_lastwritetime  = other.m_lastwritetime;
        return *this;
    }

    bool xfiletimes::operator==(const xfiletimes& other) const
    {
        return m_creationtime == other.m_creationtime && m_lastaccesstime == other.m_lastaccesstime && m_lastwritetime == other.m_lastwritetime;
    }

    bool xfiletimes::operator!=(const xfiletimes& other) const
    {
        return m_creationtime != other.m_creationtime || m_lastaccesstime != other.m_lastaccesstime || m_lastwritetime != other.m_lastwritetime;
    }
} // namespace xcore
