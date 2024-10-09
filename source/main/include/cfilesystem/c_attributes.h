#ifndef __C_FILESYSTEM_FILE_ATTRIBUTES_AND_TIMES_H__
#define __C_FILESYSTEM_FILE_ATTRIBUTES_AND_TIMES_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ctime/c_datetime.h"

namespace ncore
{
    namespace nfs
    {
        class fileattrs_t
        {
        public:
            fileattrs_t();
            fileattrs_t(const fileattrs_t&);
            fileattrs_t(bool boArchive, bool boReadonly, bool boHidden, bool boSystem);

            bool isArchive() const;
            bool isReadOnly() const;
            bool isHidden() const;
            bool isSystem() const;

            void setArchive(bool);
            void setReadOnly(bool);
            void setHidden(bool);
            void setSystem(bool);

            fileattrs_t& operator=(const fileattrs_t&);

            bool operator==(const fileattrs_t&) const;
            bool operator!=(const fileattrs_t&) const;

        private:
            ncore::u32 mFlags;
        };

        class filetimes_t
        {
        public:
            filetimes_t();
            filetimes_t(const filetimes_t&);
            filetimes_t(const datetime_t& creationTime, const datetime_t& lastAccessTime, const datetime_t& lastWriteTime);

            void getTime(datetime_t& outCreationTime, datetime_t& outLastAccessTime, datetime_t& outLastWriteTime) const;
            void getCreationTime(datetime_t&) const;
            void getLastAccessTime(datetime_t&) const;
            void getLastWriteTime(datetime_t&) const;

            void setTime(const datetime_t& creationTime, const datetime_t& lastAccessTime, const datetime_t& lastWriteTime);
            void setCreationTime(const datetime_t&);
            void setLastAccessTime(const datetime_t&);
            void setLastWriteTime(const datetime_t&);

            filetimes_t& operator=(const filetimes_t&);

            bool operator==(const filetimes_t&) const;
            bool operator!=(const filetimes_t&) const;

        private:
            datetime_t m_creationtime;
            datetime_t m_lastaccesstime;
            datetime_t m_lastwritetime;
        };
    } // namespace nfs
}; // namespace ncore

#endif
