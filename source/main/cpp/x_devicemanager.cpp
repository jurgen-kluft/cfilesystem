#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_console.h"
#include "xbase/x_runes.h"
#include "xbase/x_va_list.h"

#include "filesystem_t/x_dirpath.h"
#include "filesystem_t/x_filepath.h"
#include "filesystem_t/private/x_devicemanager.h"
#include "filesystem_t/private/x_filedevice.h"
#include "filesystem_t/private/x_path.h"

namespace xcore
{
    //------------------------------------------------------------------------------
    void devicemanager_t::clear()
    {
        for (s32 i = 0; i < mNumAliases; ++i)
		{
			mStrAlloc->deallocate(mAliasList[i].mTarget);
			mStrAlloc->deallocate(mAliasList[i].mResolved);
		}

        mNumAliases = 0;
        for (s32 i = 0; i < MAX_FILE_ALIASES; ++i)
            mAliasList[i] = alias_t();
        mNumDevices = 0;
        for (s32 i = 0; i < MAX_FILE_ALIASES; ++i)
            mDeviceList[i] = device_t();
    }

    void devicemanager_t::exit()
    {
        for (s32 i = 0; i < mNumDevices; ++i)
        {
            filedevice_t* device = mDeviceList[i].mDevice;
            if (device != nullptr)
            {
                x_DestroyFileDevice(device);
                mDeviceList[i].mDevice = nullptr;
            }
        }
        clear();
    }

	static utf32::rune sDeviceSeperatorStr[] = { uchar32(':'), uchar32('\\'), uchar32(0) };
	static utf32::crunes sDeviceSeperator(sDeviceSeperatorStr, sDeviceSeperatorStr + 2);

    //==============================================================================
    // Functions
    //==============================================================================
    devicemanager_t::devicemanager_t(runes_alloc_t* stralloc)
        : mStrAlloc(stralloc)
		, mNumAliases(0)
        , mNumDevices(0)
    {
    }

    //------------------------------------------------------------------------------

    bool devicemanager_t::add_device(const utf32::crunes& devicename, filedevice_t* device)
    {
        for (s32 i = 0; i < mNumDevices; ++i)
        {
            if (compare(mDeviceList[i].mDevName, devicename) == 0)
            {
                mDeviceList[i].mDevice = device;
                console->writeLine("INFO replaced file device for '%s'", va_list_t(va_t(devicename)));
				mNeedsResolve = true;
                return true;
            }
        }

        if (mNumDevices < MAX_FILE_DEVICES)
        {
            utf32::copy(devicename, mDeviceList[mNumDevices].mDevName);
            mDeviceList[mNumDevices].mDevice = device;
            mNumDevices++;
			mNeedsResolve = true;
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
    bool devicemanager_t::add_alias(const utf32::crunes& alias, const utf32::crunes& target)
    {
        for (s32 i = 0; i < mNumAliases; ++i)
        {
            if (utf32::compare(mAliasList[i].mAlias, alias) == 0)
            {
                utf32::copy(target, mAliasList[i].mTarget, mStrAlloc, 8);
                console->writeLine("INFO replaced alias for '%s'", va_list_t(va_t(alias)));
				mNeedsResolve = true;
                return true;
            }
        }

        if (mNumAliases < MAX_FILE_ALIASES)
        {
            utf32::copy(alias, mAliasList[mNumAliases].mAlias);
            utf32::copy(target, mAliasList[mNumAliases].mTarget, mStrAlloc, 8);
            mNumAliases++;
			mNeedsResolve = true;
            return true;
        }
        else
        {
            console->writeLine("ERROR cannot add another alias, maximum amount of aliases reached");
            return false;
        }
    }

    bool devicemanager_t::add_device(const char* devpath, filedevice_t* device)
    {
		utf32::runez<32> devpath32(devpath);
        return add_device(devpath32, device);
    }

    bool devicemanager_t::add_alias(const char* alias, const utf32::crunes& devname)
    {
		utf32::runez<32> alias32(alias);
        return add_alias(alias32, devname);
    }

    void devicemanager_t::resolve()
    {
        // "data"   - "appdir:\data\"
        // "appdir" - "c:\projects\a\"
        // resolves "data" as "c:\projects\a\data\"

        struct indexstack_t
        {
            s32 m_stack[15];
            s32 m_index;

			indexstack_t()
				: m_index(0)
			{
				for (s32 i=0; i<15; ++i)
					m_stack[i] = -1;
			}

            void reset() { m_index = 0; }
            bool empty() const { return m_index == 0; }

            void push(s32 index)
            {
				ASSERT(m_index < 15);
                m_stack[m_index] = index;
                m_index++;
            }

            s32 pop()
            {
                ASSERT(m_index > 0);
                m_index--;
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

		mNeedsResolve = false;

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
                utf32::concatenate(resolved_path, mAliasList[indexof_alias].mTarget, mStrAlloc, 8);
                while (stack.empty() == false)
                {
                    indexof_alias                   = stack.pop();
                    utf32::crunes target_path       = utf32::crunes(mAliasList[indexof_alias].mTarget);
                    utf32::crunes alias_target_path = utf32::findSelectUntilIncluded(target_path, sDeviceSeperator);
                    alias_target_path               = utf32::selectUntilEndExcludeSelection(target_path, alias_target_path);
                    utf32::concatenate(resolved_path, alias_target_path, mStrAlloc, 8);
                }

				// Cache the device index that our resolved path is referencing
				mAliasList[i].mDeviceIndex = -1;
				utf32::runes resolved_devname = utf32::findSelectUntilIncluded(resolved_path, sDeviceSeperator);
				if (!resolved_devname.is_empty())
				{
					for (s32 di = 0; di < mNumDevices; ++di)
					{
						if (compare(mDeviceList[di].mDevName, resolved_devname) == 0)
						{
							mAliasList[i].mDeviceIndex = di;
							break;
						}
					}
				}
            }
        }
    }

    s32 devicemanager_t::find_indexof_alias(const utf32::crunes& path) const
    {
        // reduce path to just the alias part
        utf32::crunes alias = utf32::findSelectUntilIncluded(path, sDeviceSeperator);
        for (s32 i = 0; i < mNumAliases; ++i)
        {
            if (compare(mAliasList[i].mAlias, alias) == 0)
            {
                return i;
            }
        }
        return -1;
    }
    s32 devicemanager_t::find_indexof_device(const utf32::crunes& path) const
    {
        // reduce path to just the device part
        utf32::crunes devname = utf32::findSelectUntilIncluded(path, sDeviceSeperator);
        for (s32 i = 0; i < mNumDevices; ++i)
        {
            if (compare(mDeviceList[i].mDevName, devname) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    //------------------------------------------------------------------------------
	bool devicemanager_t::has_device(const path_t& path)
	{
		if (mNeedsResolve)
		{
			resolve();
		}

		filedevice_t* fd = nullptr;
        utf32::runes devname = utf32::findSelectUntilIncluded(path.m_path, sDeviceSeperator);
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

    filedevice_t* devicemanager_t::find_device(const path_t& path, path_t& device_syspath)
	{
		if (mNeedsResolve)
		{
			resolve();
		}

		device_syspath = path_t(mStrAlloc);

		filedevice_t* fd = nullptr;
        utf32::runes devname = utf32::findSelectUntilIncluded(path.m_path, sDeviceSeperator);
		if (!devname.is_empty())
		{
			for (s32 i = 0; i < mNumAliases; ++i)
			{
				if (compare(mAliasList[i].mAlias, devname) == 0)
				{
					// Concatenate the path (filepath or dirpath) that the user provided to our resolved path
					utf32::runes relpath = utf32::selectUntilEndExcludeSelection(path.m_path, devname);
					utf32::concatenate(device_syspath.m_path, mAliasList[i].mResolved, relpath, device_syspath.m_alloc, 16);
					if (mAliasList[i].mDeviceIndex >= 0)
					{
						return mDeviceList[mAliasList[i].mDeviceIndex].mDevice;
					}
				}
			}
			for (s32 i = 0; i < mNumDevices; ++i)
			{
				if (compare(mDeviceList[i].mDevName, devname) == 0)
				{
					// Concatenate the path (filepath or dirpath) that the user provided to our device path
					utf32::runes relpath = utf32::selectUntilEndExcludeSelection(path.m_path, devname);
					utf32::concatenate(device_syspath.m_path, mDeviceList[i].mDevName, relpath, mStrAlloc, 16);
					return mDeviceList[i].mDevice;
				}
			}
		}
        return fd;
    }
}; // namespace xcore
