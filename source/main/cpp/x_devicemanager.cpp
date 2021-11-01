#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_console.h"
#include "xbase/x_log.h"
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
    void devicemanager_t::clear()
    {
        for (s32 i = 0; i < mNumAliases; ++i)
        {
            mContext->release_name(mAliasList[i].mTargetDeviceName);
            mContext->release_name(mAliasList[i].mResolvedDeviceName);
            mContext->release_path(mAliasList[i].mTargetPath);
            mContext->release_path(mAliasList[i].mResolvedPath);
        }

        mNumAliases = 0;
        for (s32 i = 0; i < MAX_FILE_ALIASES; ++i)
            mAliasList[i] = alias_t();
        mNumDevices = 0;
        for (s32 i = 0; i < MAX_FILE_DEVICES; ++i)
            mDeviceList[i] = device_t();
    }

    void devicemanager_t::exit()
    {
        for (s32 i = 0; i < mNumDevices; ++i)
        {
            pathdevice_t* const device = mDeviceList[i].mDevice;
            if (device != nullptr)
            {
                // Device-Instances can be shared over multiple registered devices.
                // Set all of the current device ptr's to nullptr.
                for (s32 j = 0; j < mNumDevices; ++j)
                {
                    if (device == mDeviceList[j].mDevice)
                    {
                        mDeviceList[j].mDevice = nullptr;
                    }
                }
                x_DestroyFileDevice(mContext->m_allocator, device->m_fd);
            }
        }
        clear();
    }

    static utf32::rune sDeviceSeperatorStr[] = {uchar32(':'), uchar32('\\'), uchar32(0)};
    static crunes_t    sDeviceSeperator((utf32::pcrune)sDeviceSeperatorStr, (utf32::pcrune)(sDeviceSeperatorStr + 2));

    //==============================================================================
    // Functions
    //==============================================================================
    devicemanager_t::devicemanager_t(filesysroot_t* ctxt) : mContext(ctxt), mNumAliases(0), mNumDevices(0) {}

    //------------------------------------------------------------------------------

    bool devicemanager_t::add_device(const crunes_t& devicename, filedevice_t* device)
    {
        pathname_t* pathdevicename = mContext->register_name(devicename);
        pathdevice_t* pathdevice = mContext->register_device(pathdevicename);
        for (s32 i = 0; i < mNumDevices; ++i)
        {
            if (mDeviceList[i].mDevice->m_name == pathdevicename)
            {
                mDeviceList[i].mDevice->m_fd = device;
                log_t::writeLine(log_t::INFO, "replaced file device for '%s'", va_list_t(va_t(devicename)));
                mNeedsResolve = true;
                return true;
            }
        }

        if (mNumDevices < MAX_FILE_DEVICES)
        {
            mDeviceList[mNumDevices].mDevice = pathdevice;
            mNumDevices++;
            mNeedsResolve = true;
            return true;
        }
        else
        {
            log_t::writeLine(log_t::ERROR, "cannot add another file device, maximum amount of devices reached");
            return false;
        }
    }

    // Examples:
    // 'app_dir:\' => "c:\users\john\programs\mygame\'
    // 'app_datadir:\' => "c:\users\john\programs\mygame\data\'
    // 'app_profilesdir:\' => "c:\users\john\programs\mygame\profiles\'
    // 'win_tempdir:\' => "c:\users\john\programs\mygame\temp\'
    bool devicemanager_t::add_alias(const crunes_t& alias, const crunes_t& target)
    {
        pathname_t* aliasname = mContext->register_name(alias);

        for (s32 i = 0; i < mNumAliases; ++i)
        {
            if (mAliasList[i].mAlias == aliasname)
            {
                pathname_t* targetdevicename;
                path_t* targetpath;
                if (mContext->register_directory(target, targetdevicename, targetpath))
                {
                    targetdevicename->incref();
                    targetpath->attach();

                    mContext->release_name(mAliasList[i].mTargetDeviceName);
                    mAliasList[i].mTargetDeviceName = targetdevicename;
                    mContext->release_path(mAliasList[i].mTargetPath);
                    mAliasList[i].mTargetPath = targetpath;
                    log_t::writeLine(log_t::INFO, "replaced alias for '%s'", va_list_t(va_t(alias)));
                    mNeedsResolve = true;
                    return true;
                }
                log_t::writeLine(log_t::ERROR, "cannot add invalid target path");
                return false;
            }
        }

        if (mNumAliases < MAX_FILE_ALIASES)
        {
            pathname_t* targetdevicename;
            path_t* targetpath;
            if (mContext->register_directory(target, targetdevicename, targetpath))
            {
                targetdevicename->incref();
                targetpath->attach();

                mAliasList[mNumAliases].mAlias = aliasname->incref();
                mContext->release_name(mAliasList[mNumAliases].mTargetDeviceName);
                mAliasList[mNumAliases].mTargetDeviceName = targetdevicename;
                mContext->release_path(mAliasList[mNumAliases].mTargetPath);
                mAliasList[mNumAliases].mTargetPath = targetpath;
                mNumAliases++;
                mNeedsResolve = true;
                return true;
            }
            log_t::writeLine(log_t::ERROR, "cannot add invalid target path");
            return false;
        }
        else
        {
            log_t::writeLine(log_t::ERROR, "cannot add another alias, maximum amount of aliases reached");
            return false;
        }
    }

    void devicemanager_t::resolve()
    {
        // "data"   - "appdir:\data\"
        // "appdir" - "c:\games\sudoku\"
        // resolves "data:\textures\" as "c:\games\sudoku\data\textures\"
        const s32 cStackSize = 8;
        struct indexstack_t
        {
            s32 m_stack[cStackSize];
            s32 m_index;

            indexstack_t() : m_index(0)
            {
                for (s32 i = 0; i < cStackSize; ++i)
                    m_stack[i] = -1;
            }

            void reset() { m_index = 0; }
            bool empty() const { return m_index == 0; }
            s32 length() const { return m_index; }
            s32 operator[](s32 index) const { return m_stack[index]; }

            void push(s32 index)
            {
                ASSERT(m_index < cStackSize);
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

                s32 indexof_next_alias = find_indexof_alias(mAliasList[indexof_alias].mTargetDeviceName);
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
                    s32 indexof_device = find_indexof_device(mAliasList[indexof_alias].mTargetDeviceName);
                    valid              = indexof_device >= 0;
                    break;
                }
            }

            if (valid)
            {
                // Walk the stack and concatenate the target strings into 'Resolved'
                // path_t* resolved_path = mAliasList[i].mResolvedPath;
                if (stack.length() > 1)
                {
                    s32 pathslen = 0;
                    path_t* paths[cStackSize];
                    for (s32 di = stack.length() - 1; di >= 0; --di)
                    {
                        if (pathslen == 0)
                        {
                            mAliasList[i].mResolvedDeviceName = mAliasList[stack[di]].mTargetDeviceName;
                            mAliasList[i].mDeviceIndex = find_indexof_device(mAliasList[stack[di]].mTargetDeviceName);
                        }
                        paths[pathslen++] = mAliasList[stack[di]].mTargetPath;
                    }

                    // We need to allocate a path_t* which can hold 'depth' folders
                    path_t* fullpath = nullptr;
                    mContext->register_directory(paths, pathslen, fullpath);
                    mAliasList[i].mResolvedPath = fullpath;
                }
                else
                {
                    mAliasList[i].mResolvedDeviceName = mAliasList[stack[0]].mTargetDeviceName;
                    mAliasList[i].mDeviceIndex = find_indexof_device(mAliasList[stack[0]].mTargetDeviceName);
                }
            }
        }
    }

    s32 devicemanager_t::find_indexof_alias(pathname_t* alias) const
    {
        if (alias == nullptr)
            return -1;

        for (s32 i = 0; i < mNumAliases; ++i)
        {
            if (mAliasList[i].mAlias == alias)
            {
                return i;
            }
        }
        return -1;
    }
    s32 devicemanager_t::find_indexof_device(pathname_t* devname) const
    {
        if (devname == nullptr)
            return -1;

        // reduce path to just the device part
        for (s32 i = 0; i < mNumDevices; ++i)
        {
            if (mDeviceList[i].mDevice->m_name == devname)
            {
                return i;
            }
        }
        return -1;
    }

    //------------------------------------------------------------------------------
    bool devicemanager_t::has_device(const runes_t& path)
    {
        if (mNeedsResolve)
        {
            resolve();
        }

        pathdevice_t* pd      = nullptr;
        runes_t       devname = findSelectUntilIncluded(path, sDeviceSeperator);
        if (!devname.is_empty())
        {
            pathname_t* devpathname = mContext->register_name(devname);
            for (s32 i = 0; i < mNumAliases; ++i)
            {
                if (mAliasList[i].mAlias == devpathname)
                {
                    pd = mDeviceList[mAliasList[i].mDeviceIndex].mDevice;
                    break;
                }
            }
        }
        return pd != nullptr;
    }

    pathdevice_t* devicemanager_t::find_device(pathname_t* devicename)
    {
        if (mNeedsResolve)
        {
            resolve();
        }

        pathdevice_t* fd      = nullptr;
        {
            for (s32 i = 0; i < mNumAliases; ++i)
            {
                if (mAliasList[i].mAlias == devicename)
                {
                    // Concatenate the path (filepath or dirpath) that the user provided to our resolved path
                    if (mAliasList[i].mDeviceIndex >= 0)
                    {
                        return mDeviceList[mAliasList[i].mDeviceIndex].mDevice;
                    }
                }
            }
            for (s32 i = 0; i < mNumDevices; ++i)
            {
                if (mDeviceList[i].mDevice->m_name == devicename)
                {
                    // Concatenate the path (filepath or dirpath) that the user provided to our device path
                    return mDeviceList[i].mDevice;
                }
            }
        }
        return fd;
    }
}; // namespace xcore
