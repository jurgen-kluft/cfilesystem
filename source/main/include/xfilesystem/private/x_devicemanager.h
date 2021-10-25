#ifndef __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#define __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_allocator.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/x_filesystem.h"

namespace xcore
{
    struct path_t;
    class dirpath_t;
    class filepath_t;
    class filedevice_t;
    struct filesysroot_t;

    //------------------------------------------------------------------------------
    // Description:
    //     This class maps an alias to a file device.
    //     This can act as an re-direction and is useful for defining folders
    //     as drives, remap drives etc..
    //
    //     source:\folder\filename.ext, where source = c:\temp
    //     data:\folder\filename.ext, where data = g:\
	//     dvd:\folder\filename.ext, where dvd = g:\
    //------------------------------------------------------------------------------
    class devicemanager_t
    {
        enum EConfig
        {
            MAX_FILE_ALIASES = 16,
            MAX_FILE_DEVICES = 48,
        };
        typedef runes_t      runes;
        typedef utf32::rune  rune;

    public:
        devicemanager_t(filesysroot_t* stralloc);
		
		XCORE_CLASS_PLACEMENT_NEW_DELETE

        void clear();
        void exit();

        bool add_device(const crunes_t& device_name, filedevice_t*);
        bool add_alias(const crunes_t& alias_name, const crunes_t& device_name);

        // Pass on the filepath or dirpath, e.g. 'c:\folder\subfolder\' or 'appdir:\data\texture.jpg'
		bool has_device(const runes_t& path);

        filedevice_t* find_device(const filepath_t& filepath, filepath_t& full_filepath);

        struct alias_t
        {
            inline alias_t()
                : mAlias(nullptr)
                , mTargetDeviceName(nullptr)
                , mTargetPath(nullptr)
                , mResolvedDeviceName(nullptr)
                , mResolvedPath(nullptr)
                , mDeviceIndex(-1)
            {
            }

            pathname_t* mAlias;              // "data"
            pathname_t* mTargetDeviceName;   // "appdir:\data\bin.pc\", "data:\file.txt" to "appdir:\data\bin.pc\file.txt"
            path_t*     mTargetPath;         // 
            pathname_t* mResolvedDeviceName; // "appdir:\data\bin.pc\" to "d:\project\data\bin.pc\"
            path_t*     mResolvedPath;       // 
			s32         mDeviceIndex;        // Index of the device that we resolve to
        };

        // Return index of found alias, otherwise -1
        s32 find_indexof_alias(pathname_t* path) const;
        s32 find_indexof_device(pathname_t* path) const;

		void resolve();

		struct device_t
        {
            inline device_t()
                : mDevName(nullptr)
                , mDevice(nullptr)
            {
            }
            pathname_t*   mDevName;
            filedevice_t* mDevice;
        };

		bool          mNeedsResolve;
        filesysroot_t* mContext;
        s32           mNumAliases;
        alias_t       mAliasList[MAX_FILE_ALIASES];
        s32           mNumDevices;
        device_t      mDeviceList[MAX_FILE_DEVICES];
    };
}; // namespace xcore

#endif