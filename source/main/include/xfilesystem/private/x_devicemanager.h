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
            MAX_FILE_ALIASES = 64,
            MAX_FILE_DEVICES = 64,
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

        // Pass on the filepath or dirpath, e.g. 'c:\folder\subfolder\' or 'appdir:\data\texture.jpg'
        xfiledevice* find_device(const utf32::crunes& devname);

        struct alias_t
        {
            inline alias_t()
                : mAlias(mAliasRunes, mAliasRunes, mAliasRunes + sizeof(mAliasRunes) - 1)
                , mTarget(mTargetRunes, mTargetRunes, mTargetRunes + sizeof(mTargetRunes) - 1)
            {
                mAliasRunes[sizeof(mAliasRunes) - 1]   = '\0';
                mTargetRunes[sizeof(mTargetRunes) - 1] = '\0';
            }
            rune  mAliasRunes[32];
            rune  mTargetRunes[32];
            runes mAlias;  ///< data
            runes mTarget; ///< "d:\project\data\bin.pc\", data:\file.txt to d:\project\data\bin.pc\file.txt
        };

        struct device_t
        {
            inline device_t()
                : mDevName(mDevNameRunes, mDevNameRunes, mDevNameRunes + sizeof(mDevNameRunes) - 1)
                , mDevice(nullptr)
            {
                mDevNameRunes[sizeof(mDevNameRunes) - 1] = '\0';
            }
            rune         mDevNameRunes[32];
            runes        mDevName;
            xfiledevice* mDevice;
        };

        s32     mNumAliases;
        alias_t mAliasList[MAX_FILE_ALIASES];

        s32      mNumDevices;
        device_t mDeviceList[MAX_FILE_DEVICES];
    };
};

#endif