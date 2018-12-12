//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_console.h"
#include "xbase/x_runes.h"
#include "xbase/x_va_list.h"

#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/private/x_devicemanager.h"
#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_path.h"

namespace xcore
{
	//------------------------------------------------------------------------------
	void				xdevicemanager::clear()
	{
		mNumAliases = 0;
		for (s32 i=0; i<MAX_FILE_ALIASES; ++i)
			mAliasList[i] = alias_t();
		mNumDevices = 0;
		for (s32 i=0; i<MAX_FILE_ALIASES; ++i)
			mDeviceList[i] = device_t();
	}

	void				xdevicemanager::exit()
	{
		for (s32 i=0; i<mNumDevices; ++i)
		{
			xfiledevice* device = mDeviceList[i].mDevice;
			if (device != nullptr)
			{
				x_DestroyFileDevice(device);
				mDeviceList[i].mDevice = nullptr;
			}
		}
		mNumDevices = 0;
	}

	//==============================================================================
	// Functions
	//==============================================================================
	xdevicemanager::xdevicemanager()
		: mNumAliases(0)
		, mNumDevices(0)
	{
	}

	//------------------------------------------------------------------------------

	bool				xdevicemanager::add_device(const utf32::crunes& devicename, xfiledevice* device)
	{
		for (s32 i=0; i<mNumDevices; ++i)
		{
			if (compare(mDeviceList[i].mDevName, devicename) == 0)
			{
				mDeviceList[i].mDevice = device;
				console->writeLine("INFO replaced file device for '%s'", x_va_list(x_va(devicename)));
				return true;
			}
		}

		if (mNumDevices < MAX_FILE_DEVICES)
		{
			mDeviceList[mNumDevices].mDevName.clear();
			utf32::copy(devicename, mDeviceList[mNumDevices].mDevName);
			mDeviceList[mNumDevices].mDevice = device;
			mNumDevices++;
			return true;
		}
		else
		{
			console->writeLine("ERROR cannot add another file device, maximum amount of devices reached");
			return false;
		}
	}

	// Examples:
	// 'app_dir:\' => "c:\users\john\programs\mygame\'
	// 'app_datadir:\' => "c:\users\john\programs\mygame\data\'
	// 'app_profilesdir:\' => "c:\users\john\programs\mygame\profiles\'
	// 'win_tempdir:\' => "c:\users\john\programs\mygame\temp\'

	bool				xdevicemanager::add_alias(const utf32::crunes& alias, const utf32::crunes& target)
	{
		for (s32 i=0; i<mNumAliases; ++i)
		{
			if (utf32::compare(mAliasList[i].mAlias, alias) == 0)
			{
				mAliasList[i].mTarget.clear();
				utf32::copy(target, mAliasList[i].mTarget);
				console->writeLine("INFO replaced alias for '%s'", x_va_list(x_va(alias)));
				return true;
			}
		}

		if (mNumAliases < MAX_FILE_ALIASES)
		{
			mAliasList[mNumAliases].mAlias.clear();
			utf32::copy(alias, mAliasList[mNumAliases].mAlias);
			mAliasList[mNumAliases].mTarget.clear();
			utf32::copy(target, mAliasList[mNumAliases].mTarget);
			mNumAliases++;
			return true;
		}
		else
		{
			console->writeLine("ERROR cannot add another alias, maximum amount of aliases reached");
			return false;
		}
	}

	//------------------------------------------------------------------------------

	xfiledevice*		xdevicemanager::find_device(const utf32::crunes& devicename)
	{
		utf32::rune devnamerunes[32];
		utf32::runes devname(devnamerunes, devnamerunes, devnamerunes+sizeof(devnamerunes)-1);
		utf32::copy(devicename, devname);

		for (s32 i=0; i<mNumAliases; ++i)
		{
			if (compare(mAliasList[i].mAlias, devicename) == 0)
			{
				utf32::copy(mAliasList[i].mTarget, devname);
				i = -1;	// Restart
			}
		}
		for (s32 i=0; i<mNumDevices; ++i)
		{
			if (compare(mDeviceList[i].mDevName, devname) == 0)
			{
				return mDeviceList[i].mDevice;
			}
		}

		return nullptr;
	}


};
