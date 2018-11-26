#ifndef __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#define __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xstring/x_string.h"


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

	public:
								xdevicemanager();

		void					clear();

		bool					add_device(const xstring& devname, xfiledevice* );
		bool					add_alias(const xstring& remap, const xstring& devname);

		xfiledevice*			find_device(const xfilepath& );
		xfiledevice*			find_device(const xdirpath& );

		struct alias_t
		{
			inline					device_t() : mAliasStr(), mTargetStr() {}
			xstring					mAliasStr;						///< data
			xstring					mTargetStr;						///< "d:\project\data\bin.pc\", data:\file.txt to d:\project\data\bin.pc\file.txt
		};

		struct device_t
		{
			inline					device_t() : mDevName(), mDevice(nullptr) {}
			xstring					mDevName;
			xfiledevice*			mDevice;
		};

		s32						mNumAliases;
		alias_t					mAliasList[MAX_FILE_ALIASES];

		s32						mNumDevices;
		device_t				mDeviceList[MAX_FILE_DEVICES];

	};

};

#endif