#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string.h"
#include "xbase\x_string_std.h"
#include "xtime\x_datetime.h"

#include "xfilesystem\x_dirinfo.h"

#include "xfilesystem\x_devicealias.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		static const xdevicealias* sGetAlias(const xdirpath& dirpath)
		{
			char deviceStrBuffer[64];
			xcstring deviceName(deviceStrBuffer, sizeof(deviceStrBuffer));
			dirpath.getDeviceName(deviceName);
			const xdevicealias* alias = xdevicealias::sFind( deviceName.isEmpty() ? "currdir" : deviceStrBuffer);
			return alias;
		}

		xdirinfo::xdirinfo()
		{
		}

		xdirinfo::xdirinfo(const xdirinfo& dirinfo)
			: mDirPath(dirinfo.mDirPath)
		{
		}

		xdirinfo::xdirinfo(const char* dir)
			: mDirPath(dir)
		{
		}

		xdirinfo::xdirinfo(const xdirpath& dir)
			: mDirPath(dir)
		{
		}

		const xdirpath&			xdirinfo::getFullName() const
		{
			return mDirPath;
		}

		bool					xdirinfo::getName(xcstring& outName) const
		{
			return mDirPath.getName(outName);
		}


		bool					xdirinfo::isValid() const
		{
			return sIsValid(mDirPath);
		}

		bool					xdirinfo::isRoot() const
		{
			return mDirPath.isRoot();
		}

		bool					xdirinfo::isRooted() const
		{
			return mDirPath.isRooted();
		}


		bool					xdirinfo::exists() const
		{
			return sExists(mDirPath);
		}

		bool					xdirinfo::create()
		{
			return sCreate(mDirPath);
		}

		bool					xdirinfo::remove()
		{
			return sDelete(mDirPath);
		}

		void					xdirinfo::refresh()
		{
		}


		void					xdirinfo::copy(const xdirpath& toDirectory, xbool overwrite)
		{
			sCopy(mDirPath, toDirectory, overwrite);
		}

		void					xdirinfo::move(const xdirpath& toDirectory)
		{
			sMove(mDirPath, toDirectory);
		}


		void					xdirinfo::enumerateFiles(enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories)
		{
			sEnumerateFiles(mDirPath, enumerator, searchSubDirectories);
		}

		void					xdirinfo::enumerateDirs(enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories)
		{
			sEnumerateDirs(mDirPath, enumerator, searchSubDirectories);
		}


		const xdevicealias*		xdirinfo::getAlias() const
		{
			return sGetAlias(mDirPath);
		}

		bool					xdirinfo::getRoot(xdirinfo& outRootDirPath) const
		{
			return (mDirPath.getRoot(outRootDirPath.mDirPath));
		}

		bool					xdirinfo::getParent(xdirinfo& outParentDirPath) const
		{
			return (mDirPath.getParent(outParentDirPath.mDirPath));
		}

		bool					xdirinfo::getSubdir(const char* subDir, xdirinfo& outSubDirPath) const
		{
			return (mDirPath.getSubDir(subDir, outSubDirPath.mDirPath));
		}


		bool					xdirinfo::getCreationTime  (xdatetime& outTime) const
		{
			return sGetCreationTime(mDirPath, outTime);
		}

		bool					xdirinfo::getLastAccessTime(xdatetime& outTime) const
		{
			return sGetLastAccessTime(mDirPath, outTime);
		}

		bool					xdirinfo::getLastWriteTime (xdatetime& outTime) const
		{
			return sGetLastWriteTime(mDirPath, outTime);
		}

		bool					xdirinfo::setCreationTime(const xdatetime& inTime) const
		{
			return sSetCreationTime(mDirPath, inTime);
		}

		bool					xdirinfo::setLastAccessTime(const xdatetime& inTime) const
		{
			return sSetLastAccessTime(mDirPath, inTime);
		}

		bool					xdirinfo::setLastWriteTime (const xdatetime& inTime) const
		{
			return sSetLastWriteTime(mDirPath, inTime);
		}

		xdirinfo&				xdirinfo::operator = (const char* other)
		{
			mDirPath = other;
			return *this;
		}

		xdirinfo&				xdirinfo::operator = (const xdirinfo& other)
		{
			if (this == &other)
				return *this;

			mDirPath = other.mDirPath;
			return *this;
		}

		xdirinfo&				xdirinfo::operator = (const xdirpath& other)
		{
			if (&mDirPath == &other)
				return *this;

			mDirPath = other;
			return *this;
		}

		bool					xdirinfo::operator == (const char* other) const
		{
			return mDirPath == other;
		}

		bool					xdirinfo::operator != (const char* other) const
		{
			return mDirPath != other;
		}

		bool					xdirinfo::operator == (const xdirpath& other) const
		{
			return mDirPath == other;
		}

		bool					xdirinfo::operator != (const xdirpath& other) const
		{
			return mDirPath != other;
		}

		bool					xdirinfo::operator == (const xdirinfo& other) const
		{
			return mDirPath == other.mDirPath;
		}

		bool					xdirinfo::operator != (const xdirinfo& other) const
		{
			return mDirPath != other.mDirPath;
		}


		///< Static functions
		bool					xdirinfo::sIsValid(const xdirpath& directory)
		{
			char deviceStrBuffer[64];
			xcstring deviceName(deviceStrBuffer, sizeof(deviceStrBuffer));
			directory.getDeviceName(deviceName);
			if (deviceName.isEmpty())
				return true;
			const xdevicealias* _alias = xdevicealias::sFind(deviceName.c_str());
			return (_alias != NULL);
		}

		bool					xdirinfo::sCreate(const xdirpath& directory)
		{
			const xdevicealias* _alias = sGetAlias(directory);
			
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			directory.getSystem(systemDir);

			if (_alias!=NULL && _alias->device()->createDir(systemDir.c_str()))
			{
				return true;
			}
			return false;
		}

		bool					xdirinfo::sDelete(const xdirpath& directory)
		{
			const xdevicealias* _alias = sGetAlias(directory);

			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			directory.getSystem(systemDir);

			return _alias!=NULL && _alias->device()->deleteDir(systemDir.c_str());
		}

		bool					xdirinfo::sExists(const xdirpath& directory)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			return device!=NULL && device->hasDir(systemDir.c_str());
		}

		void					xdirinfo::sEnumerate(const xdirpath& directory, enumerate_delegate<xfileinfo>& file_enumerator, enumerate_delegate<xdirinfo>& dir_enumerator, bool searchSubDirectories)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL)
				device->enumerate(directory.c_str(), searchSubDirectories, &file_enumerator, &dir_enumerator);
		}

		void					xdirinfo::sEnumerateFiles(const xdirpath& directory, enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL)
				device->enumerate(systemDir.c_str(), searchSubDirectories, &enumerator, NULL);
		}

		void					xdirinfo::sEnumerateDirs(const xdirpath& directory, enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL)
				device->enumerate(systemDir.c_str(), searchSubDirectories, NULL, &enumerator);
		}

		bool					xdirinfo::sSetTime(const xdirpath& directory, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL)
				return device->setDirTime(systemDir.c_str(), creationTime, lastAccessTime, lastWriteTime);
			return false;
		}

		bool					xdirinfo::sGetTime(const xdirpath& directory, xdatetime& creationTime, xdatetime& lastAccessTime, xdatetime& lastWriteTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL && device->getDirTime(systemDir.c_str(), creationTime, lastAccessTime, lastWriteTime))
				return true;
			creationTime   = xdatetime::sMinValue;
			lastAccessTime = xdatetime::sMinValue;
			lastWriteTime  = xdatetime::sMinValue;
			return false;
		}

		bool					xdirinfo::sSetCreationTime(const xdirpath& directory, const xdatetime& creationTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL)
			{
				xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
				if (device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
					if (device->setDirTime(systemDir.c_str(), creationTime, _lastAccessTime, _lastWriteTime))
						return true;
			}
			return false;
		}

		bool					xdirinfo::sGetCreationTime(const xdirpath& directory, xdatetime& outCreationTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			xdatetime _lastAccessTime, _lastWriteTime;
			if (device!=NULL && device->getDirTime(systemDir.c_str(), outCreationTime, _lastAccessTime, _lastWriteTime))
				return true;
			outCreationTime = xdatetime::sMinValue;
			return false;
		}

		bool					xdirinfo::sSetLastAccessTime(const xdirpath& directory, const xdatetime& lastAccessTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL)
			{
				xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
				if (device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
					if (device->setDirTime(systemDir.c_str(), _creationTime, lastAccessTime, _lastWriteTime))
						return true;
			}
			return false;
		}

		bool					xdirinfo::sGetLastAccessTime(const xdirpath& directory, xdatetime& outLastAccessTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			xdatetime _creationTime, _lastWriteTime;
			if (device!=NULL && device->getDirTime(systemDir.c_str(), _creationTime, outLastAccessTime, _lastWriteTime))
				return true;
			outLastAccessTime = xdatetime::sMinValue;
			return false;
		}

		bool					xdirinfo::sSetLastWriteTime(const xdirpath& directory, const xdatetime& lastWriteTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			if (device!=NULL)
			{
				xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
				if (device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
					if (device->setDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, lastWriteTime))
						return true;
			}
			return false;
		}

		bool					xdirinfo::sGetLastWriteTime(const xdirpath& directory, xdatetime& outLastWriteTime)
		{
			char systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring systemDir(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = directory.getSystem(systemDir);
			xdatetime _creationTime, _lastAccessTime;
			if (device!=NULL && device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, outLastWriteTime))
				return true;
			outLastWriteTime = xdatetime::sMinValue;
			return false;
		}


		bool					xdirinfo::sCopy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, xbool overwrite)
		{
			char srcSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring srcSystemDir(srcSystemDirBuffer, sizeof(srcSystemDirBuffer));
			xfiledevice* src_device = sourceDirectory.getSystem(srcSystemDir);
			char dstSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring dstSystemDir(dstSystemDirBuffer, sizeof(dstSystemDirBuffer));
			xfiledevice* dst_device = destDirectory.getSystem(dstSystemDir);

			if (src_device!=NULL && dst_device!=NULL)
				return src_device->copyDir(srcSystemDir.c_str(), dstSystemDir.c_str(), overwrite);

			return false;
		}

		bool					xdirinfo::sMove(const xdirpath& sourceDirectory, const xdirpath& destDirectory)
		{
			char srcSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring srcSystemDir(srcSystemDirBuffer, sizeof(srcSystemDirBuffer));
			xfiledevice* src_device = sourceDirectory.getSystem(srcSystemDir);
			char dstSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring dstSystemDir(dstSystemDirBuffer, sizeof(dstSystemDirBuffer));
			xfiledevice* dst_device = destDirectory.getSystem(dstSystemDir);

			if (src_device!=NULL && dst_device!=NULL)
				return src_device->moveDir(srcSystemDir.c_str(), dstSystemDir.c_str());

			return false;
		}

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILEPATH_H__
//==============================================================================
