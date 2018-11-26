//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"

#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/private/x_devicemanager.h"
#include "xfilesystem/private/x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
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

	//==============================================================================
	// Functions
	//==============================================================================
	xdevicemanager::xdevicemanager()
		: mNumAliases(0)
		, mNumDevices(0)
	{
	}

	//------------------------------------------------------------------------------

	bool				xdevicemanager::add(const xstring& devname, xfiledevice* device)
	{
		for (s32 i=0; i<mNumDevices; ++i)
		{
			if (compare(mDeviceList[i].mDevName, devname) == 0)
			{
				mDeviceList[i].mDevice = device;
				xconsole::writeLine("INFO replaced file device for '%s'", x_va_list(devname));
				return true;
			}
		}

		if (mNumDevices < MAX_FILE_DEVICES)
		{
			mDeviceList[mNumDevices].mDevName = devname;
			mDeviceList[mNumDevices].mDevice = device;
			mNumDevices++;
			return true;
		}
		else
		{
			xconsole::writeLine("ERROR cannot add another file device, maximum amount of devices reached");
			return false;
		}
	}

	bool				xdevicemanager::add(const xstring& alias, const xstring& target)
	{
		for (s32 i=0; i<mNumAliases; ++i)
		{
			if (compare(mAliasList[i].mAliasStr, alias) == 0)
			{
				mAliasList[i].mTargetStr = target;
				xconsole::writeLine("INFO replaced alias for '%s'", x_va_list(alias));
				return true;
			}
		}

		if (mNumAliases < MAX_FILE_ALIASES)
		{
			mAliasList[mNumAliases].mAliasStr = alias;
			mAliasList[mNumAliases].mTargetStr = target;
			mNumAliases++;
			return true;
		}
		else
		{
			xconsole::writeLine("ERROR cannot add another alias, maximum amount of aliases reached");
			return false;
		}
	}

	//------------------------------------------------------------------------------

	xfiledevice*		xdevicemanager::find_device(const xfilepath& fp)
	{
		xstring devname;
		if (fp.getDeviceName(devname))
		{
			for (s32 i=0; i<mNumAliases; ++i)
			{
				if (compare(mAliasList[i].mAliasStr, devname) == 0)
				{
					devname = mAliasList[i].mTargetStr;
					i = -1;
				}
			}
			for (s32 i=0; i<mNumDevices; ++i)
			{
				if (compare(mDeviceList[i].mDevName, devname) == 0)
				{
					return mDeviceList[i].mDevice;
				}
			}
		}
		return NULL;
	}

	//------------------------------------------------------------------------------

	xfiledevice*		xdevicemanager::find_device(const xdirpath& dp)
	{
		xstring devname;
		if (dp.getDeviceName(devname))
		{
			for (s32 i=0; i<mNumAliases; ++i)
			{
				if (compare(mAliasList[i].mAliasStr, devname) == 0)
				{
					devname = mAliasList[i].mTargetStr;
					i = -1;
				}
			}
			for (s32 i=0; i<mNumDevices; ++i)
			{
				if (compare(mDeviceList[i].mDevName, devname) == 0)
				{
					return mDeviceList[i].mDevice;
				}
			}
		}
		return NULL;
	}


};
