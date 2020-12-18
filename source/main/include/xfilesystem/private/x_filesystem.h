#ifndef __X_FILESYSTEM_FILESYSTEM_IMP_H__
#define __X_FILESYSTEM_FILESYSTEM_IMP_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_allocator.h"
#include "xbase/x_buffer.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
    class datetime_t;

    class file_t;
	class filesys_t;
    class filedevice_t;
    class devicemanager_t;
    class fileinfo_t;
    class dirinfo_t;
    class path_t;
    class filepath_t;
    class dirpath_t;
    class fileattrs_t;
    class filetimes_t;
    class stream_t;
    class istream_t;
    
    class filesys_t
    {
    public:
        char            m_slash;
        alloc_t*         m_allocator;
        runes_alloc_t*   m_stralloc;
        devicemanager_t* m_devman;

		XCORE_CLASS_PLACEMENT_NEW_DELETE

        static istream_t*	create_filestream(const filepath_t& filepath, EFileMode, EFileAccess, EFileOp);
        static void			destroy(istream_t* stream);
        static filepath_t    resolve(filepath_t const&, filedevice_t*& device);
        static dirpath_t     resolve(dirpath_t const&, filedevice_t*& device);
        static path_t&       get_xpath(dirinfo_t& dirinfo);
        static path_t const& get_xpath(dirinfo_t const& dirinfo);
        static path_t&       get_xpath(dirpath_t& dirpath);
        static path_t const& get_xpath(dirpath_t const& dirpath);
        static path_t&       get_xpath(filepath_t& filepath);
        static path_t const& get_xpath(filepath_t const& filepath);
        static filesys_t*    get_filesystem(dirpath_t const& dirpath);
        static filesys_t*    get_filesystem(filepath_t const& filepath);

        // -----------------------------------------------------------
        filepath_t  filepath(const char* str);
		dirpath_t   dirpath(const char* str);
        filepath_t  filepath(const crunes_t& str);
		dirpath_t   dirpath(const crunes_t& str);
        void       to_ascii(filepath_t const& fp, runes_t& str);
		void       to_ascii(dirpath_t const& dp, runes_t& str);

		bool       register_device(const crunes_t& device_name, filedevice_t* device);

        file_t*     open(filepath_t const& filename, EFileMode mode);
        file_t*     open(fileinfo_t*, EFileMode mode);
		stream_t*   open_stream(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op);
		writer_t*   writer(file_t*);
        reader_t*   reader(file_t*);
        void       close(file_t*);
        void       close(fileinfo_t*);
        void       close(dirinfo_t*);
        void       close(reader_t*);
        void       close(writer_t*);
        void       close(stream_t*);
        fileinfo_t* info(filepath_t const& path);
        dirinfo_t*  info(dirpath_t const& path);
        bool       exists(fileinfo_t*);
        bool       exists(dirinfo_t*);
        s64        size(fileinfo_t*);
        void       rename(fileinfo_t*, filepath_t const&);
        void       move(fileinfo_t* src, fileinfo_t* dst);
        void       copy(fileinfo_t* src, fileinfo_t* dst);
        void       rm(fileinfo_t*);
        void       rm(dirinfo_t*);
        s32        read(reader_t*, buffer_t&);
        s32        write(writer_t*, cbuffer_t const&);
        void       read_async(reader_t*, buffer_t&);
        s32        wait_async(reader_t*);
    };

}; // namespace xcore

#endif