#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{
    /*

        - troot_t       roots[]
        - tpath_t       paths[]
        - tdir_t        dirs[]
        - tfilename_t   filenames[]
        - textension_t  extensions[]

        struct troot_t
        {
            utf32::rune     m_runes[32];
            utf32::runes    m_name;
            xfiledevice*    m_device;
            tpath_t         m_path;
        };

        struct tdir_t
        {
            s32             m_refs;
            utf32::runes    m_name;
        };

        struct tpath_t
        {
            s32             m_refs;
            s32             m_dirs_cnt;
            tdir_t**        m_dirs;
        };

        struct tfilename_t
        {
            s32             m_refs;
            utf32::runes    m_name;
        };

        struct textension_t
        {
            s32             m_refs;
            utf32::runes    m_name;
        };

        // This class becomes like this and as such will always be the same
        // size. If we also treat all of the tentries and copy-on-write we
        // can copy this class and share resources.
        class xfilepath
        {
        public:

        private:
            troot_t*        m_root;             // troot_t global("") if root == ""
            tpath_t*        m_path;             // tpath_t global("") if path == ""
            tfilename_t*    m_filename;         // Should always have a filename
            textension_t*   m_extension;        // textension_t global("") if ext == ""
        };

    */
};