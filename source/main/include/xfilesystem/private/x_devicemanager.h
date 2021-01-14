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
    class path_t;
    class dirpath_t;
    class filepath_t;
    class filedevice_t;

    //------------------------------------------------------------------------------
    // Description:
    //     This class maps an alias to a file device.
    //     This can act as an re-direction and is useful for defining folders
    //     as drives, remap drives etc..
    //
    //     source:\folder\filename.ext, where source = c:\temp
    //     data:\folder\filename.ext, where data = g:\
	//     dvd:\folder\filename.ext, where dvd = g:\
	//
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
        devicemanager_t(filesystem_t::context_t* stralloc);
		
		XCORE_CLASS_PLACEMENT_NEW_DELETE

        void clear();
        void exit();

        bool add_device(const char* device_name, filedevice_t*);
        bool add_alias(const char* alias_name, const crunes_t& alias_target);

        bool add_device(const crunes_t& device_name, filedevice_t*);
        bool add_alias(const crunes_t& alias_name, const crunes_t& device_name);

        // Pass on the filepath or dirpath, e.g. 'c:\folder\subfolder\' or 'appdir:\data\texture.jpg'
		bool has_device(const path_t& path);
        filedevice_t* find_device(const path_t& path, path_t& device_rootpath);

        struct alias_t
        {
            inline alias_t()
                : mAlias(mAliasRunes, mAliasRunes, mAliasRunes + (sizeof(mAliasRunes)/sizeof(mAliasRunes[0])) - 1)
                , mTarget()
                , mResolved()
                , mDeviceIndex(-1)
            {
                mAliasRunes[(sizeof(mAliasRunes) / sizeof(mAliasRunes[0])) - 1] = '\0';
            }
            rune  mAliasRunes[16]; // 
            runes mAlias;          // "data"
            runes mTarget;         // "appdir:\data\bin.pc\", "data:\file.txt" to "appdir:\data\bin.pc\file.txt"
            runes mResolved;       // "appdir:\data\bin.pc\" to "d:\project\data\bin.pc\"
			s32   mDeviceIndex;    // Index of the device that we resolve to
        };

        // Return index of found alias, otherwise -1
        s32 find_indexof_alias(const crunes_t& path) const;
        s32 find_indexof_device(const crunes_t& path) const;

		void resolve();

		struct device_t
        {
            inline device_t()
                : mDevName(mDevNameRunes, mDevNameRunes, mDevNameRunes + (sizeof(mDevNameRunes) / sizeof(mDevNameRunes[0])) - 1)
                , mDevice(nullptr)
            {
                mDevNameRunes[(sizeof(mDevNameRunes) / sizeof(mDevNameRunes[0])) - 1] = '\0';
            }
            rune         mDevNameRunes[16];
            runes        mDevName;
            filedevice_t* mDevice;
        };

		bool          mNeedsResolve;
        filesystem_t::context_t* mContext;
        s32           mNumAliases;
        alias_t       mAliasList[MAX_FILE_ALIASES];
        s32           mNumDevices;
        device_t      mDeviceList[MAX_FILE_DEVICES];
    };
}; // namespace xcore

#endif