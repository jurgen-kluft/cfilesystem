#include "ccore/c_target.h"
#include "ccore/c_debug.h"

#include "cfilesystem/c_attributes.h"
namespace ncore
{
    enum EAttributes
    {
        FILE_ATTRIBUTE_ARCHIVE  = 1,
        FILE_ATTRIBUTE_READONLY = 2,
        FILE_ATTRIBUTE_HIDDEN   = 4,
        FILE_ATTRIBUTE_SYSTEM   = 8,
    };

    fileattrs_t::fileattrs_t() : mFlags(0) {}

    fileattrs_t::fileattrs_t(const fileattrs_t& other) : mFlags(other.mFlags) {}

    fileattrs_t::fileattrs_t(bool boArchive, bool boReadonly, bool boHidden, bool boSystem) : mFlags(0)
    {
        setArchive(boArchive);
        setReadOnly(boReadonly);
        setHidden(boHidden);
        setSystem(boSystem);
    }

    bool fileattrs_t::isArchive() const { return (mFlags & FILE_ATTRIBUTE_ARCHIVE) != 0; }
    bool fileattrs_t::isReadOnly() const { return (mFlags & FILE_ATTRIBUTE_READONLY) != 0; }
    bool fileattrs_t::isHidden() const { return (mFlags & FILE_ATTRIBUTE_HIDDEN) != 0; }
    bool fileattrs_t::isSystem() const { return (mFlags & FILE_ATTRIBUTE_SYSTEM) != 0; }

    void fileattrs_t::setArchive(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_ARCHIVE;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_ARCHIVE;
    }

    void fileattrs_t::setReadOnly(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_READONLY;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_READONLY;
    }

    void fileattrs_t::setHidden(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_HIDDEN;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_HIDDEN;
    }

    void fileattrs_t::setSystem(bool enable)
    {
        if (enable)
            mFlags = mFlags | FILE_ATTRIBUTE_SYSTEM;
        else
            mFlags = mFlags & ~FILE_ATTRIBUTE_SYSTEM;
    }

    fileattrs_t& fileattrs_t::operator=(const fileattrs_t& other)
    {
        mFlags = other.mFlags;
        return *this;
    }

    bool fileattrs_t::operator==(const fileattrs_t& other) const { return mFlags == other.mFlags; }
    bool fileattrs_t::operator!=(const fileattrs_t& other) const { return mFlags != other.mFlags; }

    filetimes_t::filetimes_t() {}
    filetimes_t::filetimes_t(const filetimes_t& ftimes) : m_creationtime(ftimes.m_creationtime), m_lastaccesstime(ftimes.m_lastaccesstime), m_lastwritetime(ftimes.m_lastwritetime) {}

    filetimes_t::filetimes_t(const datetime_t& creationTime, const datetime_t& lastAccessTime, const datetime_t& lastWriteTime) : m_creationtime(creationTime), m_lastaccesstime(lastAccessTime), m_lastwritetime(lastWriteTime) {}

    void filetimes_t::getTime(datetime_t& outCreationTime, datetime_t& outLastAccessTime, datetime_t& outLastWriteTime) const
    {
        outCreationTime   = m_creationtime;
        outLastAccessTime = m_lastaccesstime;
        outLastWriteTime  = m_lastwritetime;
    }

    void filetimes_t::getCreationTime(datetime_t& dt) const { dt = m_creationtime; }
    void filetimes_t::getLastAccessTime(datetime_t& dt) const { dt = m_lastaccesstime; }
    void filetimes_t::getLastWriteTime(datetime_t& dt) const { dt = m_lastwritetime; }

    void filetimes_t::setTime(const datetime_t& creationTime, const datetime_t& lastAccessTime, const datetime_t& lastWriteTime)
    {
        m_creationtime   = creationTime;
        m_lastaccesstime = lastAccessTime;
        m_lastwritetime  = lastWriteTime;
    }

    void filetimes_t::setCreationTime(const datetime_t& dt) { m_creationtime = dt; }
    void filetimes_t::setLastAccessTime(const datetime_t& dt) { m_lastaccesstime = dt; }
    void filetimes_t::setLastWriteTime(const datetime_t& dt) { m_lastwritetime = dt; }

    filetimes_t& filetimes_t::operator=(const filetimes_t& other)
    {
        m_creationtime   = other.m_creationtime;
        m_lastaccesstime = other.m_lastaccesstime;
        m_lastwritetime  = other.m_lastwritetime;
        return *this;
    }

    bool filetimes_t::operator==(const filetimes_t& other) const { return m_creationtime == other.m_creationtime && m_lastaccesstime == other.m_lastaccesstime && m_lastwritetime == other.m_lastwritetime; }

    bool filetimes_t::operator!=(const filetimes_t& other) const { return m_creationtime != other.m_creationtime || m_lastaccesstime != other.m_lastaccesstime || m_lastwritetime != other.m_lastwritetime; }
} // namespace ncore
