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
    void xdevicemanager::clear()
    {
        mNumAliases = 0;
        for (s32 i = 0; i < MAX_FILE_ALIASES; ++i)
            mAliasList[i] = alias_t();
        mNumDevices = 0;
        for (s32 i = 0; i < MAX_FILE_ALIASES; ++i)
            mDeviceList[i] = device_t();
    }

    void xdevicemanager::exit()
    {
        for (s32 i = 0; i < mNumDevices; ++i)
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

    bool xdevicemanager::add_device(const utf32::crunes& devicename, xfiledevice* device)
    {
        for (s32 i = 0; i < mNumDevices; ++i)
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
    bool xdevicemanager::add_alias(const utf32::crunes& alias, const utf32::crunes& target)
    {
        for (s32 i = 0; i < mNumAliases; ++i)
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

    bool xdevicemanager::add_device(const char* devname, xfiledevice* device)
    {
        rune  devname_[32];
        runes devname32(devname_, devname_, devname_ + 31);
        utf::copy(devname, devname32);
        return add_device(devname32, device);
    }

    bool xdevicemanager::add_alias(const char* alias, const utf32::crunes& devname)
    {
        rune  alias_[32];
        runes alias32(alias_, alias_, alias_ + 31);
        utf::copy(devname, alias32);
        return add_alias(alias32, devname);
    }

    void xdevicemanager::resolve()
    {
        // "data"   - "appdir:\data\"
        // "appdir" - "c:\projects\a\"
        // resolves "data" as "c:\projects\a\data\"

        struct indexstack_t
        {
            s32 m_stack[8];
            s32 m_index;

            void reset() { m_index = 0; }
            bool empty() const { return m_index == 0; }

            void push(s32 index)
            {
                m_stack[m_index] = index;
                m_index++;
            }

            s32 pop()
            {
                if (m_index > 0)
                {
                    m_index -= 1;
                }
                s32 index = m_stack[m_index];
                return index;
            }

            bool index_exists(s32 index) const
            {
                bool exists = false;
                for (s32 i = 0; i < m_index && !exists; i++)
                {
                    exists = (m_stack[i] == index);
                }
                return exists;
            }
        };

        indexstack_t stack;
        for (s32 i = 0; i < mNumAliases; ++i)
        {
            s32  indexof_alias = i;
            bool valid         = true;
            while (valid)
            {
                stack.push(indexof_alias);
                s32 indexof_next_alias = find_indexof_alias(mAliasList[indexof_alias].mTarget);
                if (indexof_next_alias >= 0)
                {
                    // Target is directing us to another alias, so we need to
                    // go and find the next alias. First store the current
                    // alias index.
                    if (!stack.index_exists(indexof_next_alias))
                    {
                        indexof_alias = indexof_next_alias;
                    }
                    else
                    {
                        valid = false; // We identified an alias looping issue
                    }
                }
                else
                {
                    // Target is likely directing us to a device, see if that
                    // is true. If it is not true then we should store a '-1'
                    // to indicate on the stack that there is a problem.
                    s32 indexof_device = find_indexof_device(mAliasList[indexof_alias].mTarget);
                    valid              = indexof_device >= 0;
                    break;
                }
            }

            if (valid)
            {
                // Walk the stack and concatenate the target strings into 'Resolved'
				utf32::runes& resolved_path = mAliasList[i].mResolved;
                resolved_path.reset();

                s32 indexof_alias = stack.pop();
                utf32::concatenate(resolved_path, mAliasList[indexof_alias].mTarget);
                while (stack.empty() == false)
                {
                    indexof_alias                   = stack.pop();
                    utf32::crunes target_path       = utf32::crunes(mAliasList[indexof_alias].mTarget);
                    utf32::crunes alias_target_path = utf32::find(target_path, ':');
                    alias_target_path               = utf32::selectUntilEndExcludeSelection(target_path, alias_target_path);
                    utf32::concatenate(resolved_path, alias_target_path);
                }

				// Cache the device index that our resolved path is referencing
				mAliasList[i].mDeviceIndex = -1;
				utf32::runes resolved_devname = utf32::findSelectUntil(resolved_path, ':');
				if (!resolved_devname.is_empty())
				{
					for (s32 di = 0; di < mNumDevices; ++di)
					{
						if (compare(mDeviceList[di].mDevName, resolved_devname) == 0)
						{
							mAliasList[i].mDeviceIndex = di;
						}
					}
				}

            }
        }
    }

    s32 xdevicemanager::find_indexof_alias(const utf32::crunes& path) const
    {
        // reduce path to just the alias part
        utf32::crunes alias = utf32::find(utf32::crunes(path), ':');
        for (s32 i = 0; i < mNumAliases; ++i)
        {
            if (compare(mAliasList[i].mAlias, alias) == 0)
            {
                return i;
            }
        }
        return -1;
    }
    s32 xdevicemanager::find_indexof_device(const utf32::crunes& path) const
    {
        // reduce path to just the device part
        utf32::crunes device = utf32::find(utf32::crunes(path), ':');
        for (s32 i = 0; i < mNumDevices; ++i)
        {
            if (compare(mDeviceList[i].mDevName, device) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    //------------------------------------------------------------------------------
	bool xdevicemanager::has_device(const xpath& path) const
	{
		xfiledevice* fd = nullptr;
        utf32::runes devname = utf32::findSelectUntil(path.m_path, ':');
		if (!devname.is_empty())
		{
			for (s32 i = 0; i < mNumAliases; ++i)
			{
				if (compare(mAliasList[i].mAlias, devname) == 0)
				{
					fd = mDeviceList[mAliasList[i].mDeviceIndex].mDevice;
					break;
				}
			}
		}
        return fd != nullptr;
	}

    xfiledevice* xdevicemanager::find_device(const xpath& path, xpath& device_syspath)
	{
		xfiledevice* fd = nullptr;

        utf32::runes devname = utf32::findSelectUntil(path.m_path, ':');
		if (!devname.is_empty())
		{
			for (s32 i = 0; i < mNumAliases; ++i)
			{
				if (compare(mAliasList[i].mAlias, devname) == 0)
				{
					device_syspath = mAliasList[i].mResolved;

					// Concatenate the path (filepath or dirpath) that the user provided to our resolved path
					utf32::runes relpath = utf32::selectUntilEndExcludeSelection(path.m_path, devname);
					utf32::trimLeft(relpath, ':');
					utf32::trimLeft(relpath, '\\');
					device_syspath += relpath;
					if (mAliasList[i].mDeviceIndex >= 0)
					{
						fd = mDeviceList[mAliasList[i].mDeviceIndex].mDevice;
					}
					break;
				}
			}
		}
        return fd;
    }
}; // namespace xcore
