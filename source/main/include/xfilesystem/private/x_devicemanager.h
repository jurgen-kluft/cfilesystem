#ifndef __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#define __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xfilesystem/private/x_path.h"

namespace xcore
{
    class xdirpath;
    class xfilepath;
    class xfiledevice;

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
    class xdevicemanager
    {
        enum EConfig
        {
            MAX_FILE_ALIASES = 16,
            MAX_FILE_DEVICES = 48,
        };
        typedef utf32::runes runes;
        typedef utf32::rune  rune;

    public:
        xdevicemanager();

        void clear();
        void exit();

        bool add_device(const char* devname, xfiledevice*);
        bool add_alias(const char* remap, const utf32::crunes& devname);

        bool add_device(const utf32::crunes& devname, xfiledevice*);
        bool add_alias(const utf32::crunes& remap, const utf32::crunes& devname);

        void resolve();

        // Pass on the filepath or dirpath, e.g. 'c:\folder\subfolder\' or 'appdir:\data\texture.jpg'
        xfiledevice* find_device(const utf32::crunes& devname, utf32::crunes& device_path);

        struct alias_t
        {
            inline alias_t()
                : mAlias(mAliasRunes, mAliasRunes, mAliasRunes + sizeof(mAliasRunes) - 1)
                , mTarget(mTargetRunes, mTargetRunes, mTargetRunes + sizeof(mTargetRunes) - 1)
                , mResolved(mResolvedRunes, mResolvedRunes, mResolvedRunes + sizeof(mResolvedRunes) - 1)
            {
                mAliasRunes[sizeof(mAliasRunes) - 1]       = '\0';
                mTargetRunes[sizeof(mTargetRunes) - 1]     = '\0';
                mResolvedRunes[sizeof(mResolvedRunes) - 1] = '\0';
            }
            rune  mAliasRunes[16];
            rune  mTargetRunes[112];
            rune  mResolvedRunes[128];
            runes mAlias;    // "data"
            runes mTarget;   // "appdir:\data\bin.pc\", "data:\file.txt" to "appdir:\data\bin.pc\file.txt"
            runes mResolved; // "appdir:\data\bin.pc\" to "d:\project\data\bin.pc\"
        };

        // Return index of found alias, otherwise -1
        s32 find_indexof_alias(const utf32::crunes& path) const;
        s32 find_indexof_device(const utf32::crunes& path) const;

        struct device_t
        {
            inline device_t()
                : mDevName(mDevNameRunes, mDevNameRunes, mDevNameRunes + sizeof(mDevNameRunes) - 1)
                , mDevice(nullptr)
            {
                mDevNameRunes[sizeof(mDevNameRunes) - 1] = '\0';
            }
            rune         mDevNameRunes[16];
            runes        mDevName;
            xfiledevice* mDevice;
        };

        s32     mNumAliases;
        alias_t mAliasList[MAX_FILE_ALIASES];

        s32      mNumDevices;
        device_t mDeviceList[MAX_FILE_DEVICES];
    };
}; // namespace xcore

#endif