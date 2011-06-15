#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_limits.h"
#include "xbase\x_debug.h"
#include "xbase\x_string.h"
#include "xbase\x_string_std.h"
#include "xtime\x_datetime.h"

#include "xfilesystem\x_attributes.h"
#include "xfilesystem\x_dirinfo.h"
#include "xfilesystem\x_devicealias.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_fileinfo.h"
#include "xfilesystem\x_filestream.h"
#include "xfilesystem\private\x_filesystem_common.h"


//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		static const xdevicealias*	sGetAlias(const xfilepath& filepath)
		{
			char deviceStrBuffer[64];
			xcstring deviceName(deviceStrBuffer, sizeof(deviceStrBuffer));
			filepath.getDeviceName(deviceName);
			const xdevicealias* alias = xdevicealias::sFind( deviceName.isEmpty() ? "currdir" : deviceName.c_str());
			return alias;
		}

		xfileinfo::xfileinfo()
		{
		}

		xfileinfo::xfileinfo(const xfileinfo& dirinfo)
			: mFilePath(dirinfo.mFilePath)
		{
		}

		xfileinfo::xfileinfo(const char* path)
			: mFilePath(path)
		{
		}

		xfileinfo::xfileinfo(const xfilepath& path)
			: mFilePath(path)
		{
		}

		const xfilepath&		xfileinfo::getFullName() const
		{
			return mFilePath;
		}

		void					xfileinfo::getName(xcstring& outName) const
		{
			return mFilePath.getName(outName);
		}

		void					xfileinfo::getExtension(xcstring& outExtension) const
		{
			return mFilePath.getExtension(outExtension);
		}

		u64						xfileinfo::getLength() const
		{
			return sGetLength(mFilePath);
		}

		void					xfileinfo::setLength(u64 length)
		{
			sSetLength(mFilePath, length);
		}

		bool					xfileinfo::isValid() const
		{
			return sGetAlias(mFilePath) != NULL;
		}

		bool					xfileinfo::isRooted() const
		{
			return mFilePath.isRooted();
		}

		bool					xfileinfo::isArchive() const
		{
			return sIsArchive(mFilePath);			
		}

		bool					xfileinfo::isReadOnly() const
		{
			return sIsReadOnly(mFilePath);			
		}

		bool					xfileinfo::isHidden() const
		{
			return sIsHidden(mFilePath);			
		}

		bool					xfileinfo::isSystem() const
		{
			return sIsSystem(mFilePath);			
		}

		bool					xfileinfo::exists() const
		{
			return sExists(mFilePath);
		}

		bool					xfileinfo::create()
		{
			xfilestream fs;
			if (sCreate(mFilePath, fs))
			{
				fs.close();
				return true;
			}

			return false;
		}

		bool					xfileinfo::create(xfilestream& outFilestream)
		{
			outFilestream.close();
			return (sCreate(mFilePath, outFilestream));
		}

		bool					xfileinfo::remove()
		{
			return sDelete(mFilePath);
		}

		void					xfileinfo::refresh()
		{
		}

		bool					xfileinfo::openRead(xfilestream& outFileStream)
		{
			return sOpenRead(mFilePath, outFileStream);
		}

		bool					xfileinfo::openWrite(xfilestream& outFileStream)
		{
			return sOpenWrite(mFilePath, outFileStream);
		}

		u64						xfileinfo::readAllBytes(xbyte* buffer, u64 count)
		{
			return sReadAllBytes(mFilePath, buffer, 0, count);
		}

		u64					xfileinfo::writeAllBytes(const xbyte* buffer, u64 count)
		{
			return sWriteAllBytes(mFilePath, buffer, 0, count);
		}

		bool					xfileinfo::copy(const xfilepath& toFilename, bool overwrite)
		{
			return sCopy(mFilePath, toFilename, overwrite);
		}

		bool					xfileinfo::move(const xfilepath& toFilename)
		{
			return sMove(mFilePath, toFilename);
		}


		const xdevicealias*		xfileinfo::getAlias() const
		{
			return sGetAlias(mFilePath);
		}

		void					xfileinfo::getPath(xdirinfo& outInfo) const
		{
			xdirpath p;
			mFilePath.getDirPath(p);
			outInfo = p;
		}

		bool					xfileinfo::getRoot(xdirinfo& outInfo) const
		{
			xdirpath p;
			if (mFilePath.getRoot(p))
			{
				outInfo = p;
				return true;
			}
			return false;
		}

		bool					xfileinfo::getParent(xdirinfo& outInfo) const
		{
			xdirpath p;
			if (mFilePath.getParent(p))
			{
				outInfo = p;
				return true;
			}
			return false;
		}

		void					xfileinfo::getSubDir(const char* subDir, xdirinfo& outInfo) const
		{
			xdirpath p;
			mFilePath.getSubDir(subDir, p);
			outInfo = p;
		}

		xfileinfo&				xfileinfo::onlyFilename()
		{
			mFilePath.onlyFilename();
			return *this;
		}

		void					xfileinfo::up()
		{
			mFilePath.up();
		}

		void					xfileinfo::down(const char* subDir)
		{
			mFilePath.down(subDir);			
		}

		bool					xfileinfo::getTime(xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{
			return sGetTime(mFilePath, outCreationTime, outLastAccessTime, outLastWriteTime);
		}

		bool					xfileinfo::getCreationTime  (xdatetime& outTime) const
		{
			return sGetCreationTime(mFilePath, outTime);
		}

		bool					xfileinfo::getLastAccessTime(xdatetime& outTime) const
		{
			return sGetLastAccessTime(mFilePath, outTime);
		}

		bool					xfileinfo::getLastWriteTime (xdatetime& outTime) const
		{
			return sGetLastWriteTime(mFilePath, outTime);
		}

		bool					xfileinfo::setTime(const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime)
		{
			return sSetTime(mFilePath, creationTime, lastAccessTime, lastWriteTime);
		}

		bool					xfileinfo::setCreationTime(const xdatetime& inTime)
		{
			return sSetCreationTime(mFilePath, inTime);
		}

		bool					xfileinfo::setLastAccessTime(const xdatetime& inTime)
		{
			return sSetLastAccessTime(mFilePath, inTime);
		}

		bool					xfileinfo::setLastWriteTime (const xdatetime& inTime)
		{
			return sSetLastWriteTime(mFilePath, inTime);
		}


		xfileinfo&				xfileinfo::operator = (const xfileinfo& other)
		{
			if (this == &other)
				return *this;

			mFilePath = other.mFilePath;
			return *this;
		}

		xfileinfo&				xfileinfo::operator = (const xfilepath& other)
		{
			if (&mFilePath == &other)
				return *this;

			mFilePath = other;
			return *this;
		}

		bool					xfileinfo::operator == (const xfileinfo& other) const
		{
			return mFilePath == other.mFilePath;
		}

		bool					xfileinfo::operator != (const xfileinfo& other) const
		{
			return mFilePath != other.mFilePath;
		}


		///< Static functions
		bool					xfileinfo::sExists(const xfilepath& filename)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);
			return device!=NULL && device->hasFile(systemFile.c_str());
		}

		bool				sGetFileAttributes(const xfilepath& filename, xattributes& outAttr)
		{
			xdevicealias const* da = sGetAlias(filename);
			if (da == NULL)
				return false;
			xfiledevice* device = da->device();
			if (device == NULL)
				return false;
			xattributes attr;
			if (device->getFileAttr(filename.c_str(), attr))
			{
				outAttr = attr;
				return true;
			}
			return false;
		}

		bool					xfileinfo::sIsArchive(const xfilepath& filename)
		{
			xattributes a;
			if (sGetFileAttributes(filename, a))
				return a.isArchive();
			return false;
		}

		bool					xfileinfo::sIsReadOnly(const xfilepath& filename)
		{
			xattributes a;
			if (sGetFileAttributes(filename, a))
				return a.isReadOnly();
			return false;
		}

		bool					xfileinfo::sIsHidden(const xfilepath& filename)
		{
			xattributes a;
			if (sGetFileAttributes(filename, a))
				return a.isHidden();
			return false;
		}

		bool					xfileinfo::sIsSystem(const xfilepath& filename)
		{
			xattributes a;
			if (sGetFileAttributes(filename, a))
				return a.isSystem();
			return false;
		}

		bool					xfileinfo::sCreate(const xfilepath& filename, xfilestream& outFileStream)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			u32 nFileHandle;
			if (device!=NULL && device->createFile(systemFile.c_str(), true, true, nFileHandle))
				return true;
			return false;
		}

		bool					xfileinfo::sDelete(const xfilepath& filename)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);
			return device!=NULL && device->deleteFile(systemFile.c_str());
		}


		u64						xfileinfo::sGetLength(const xfilepath& filename)
		{
			u64 fileLength = X_U64_MAX;

			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			if (device!=NULL)
			{
				u32 nFileHandle;
				if (device->openFile(systemFile.c_str(), true, false, nFileHandle))
				{
					device->getLengthOfFile(nFileHandle, fileLength);
					device->closeFile(nFileHandle);
				}
			}
			return fileLength;
		}

		void					xfileinfo::sSetLength(const xfilepath& filename, u64 length)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			if (device!=NULL)
			{
				u32 nFileHandle;
				if (device->openFile(systemFile.c_str(), true, false, nFileHandle))
				{
					device->setLengthOfFile(nFileHandle, length);
					device->closeFile(nFileHandle);
				}
			}
		}

		bool					xfileinfo::sOpen(const xfilepath& filename, xfilestream& outFileStream)
		{
			outFileStream = xfilestream(filename, FileMode_Open, FileAccess_ReadWrite, FileOp_Sync);
			return outFileStream.isOpen();
		}

		bool					xfileinfo::sOpenRead(const xfilepath& filename, xfilestream& outFileStream)
		{
			outFileStream = xfilestream(filename, FileMode_Open, FileAccess_Read, FileOp_Sync);
			return outFileStream.isOpen();
		}

		bool					xfileinfo::sOpenWrite(const xfilepath& filename, xfilestream& outFileStream)
		{
			outFileStream = xfilestream(filename, FileMode_Open, FileAccess_Write, FileOp_Sync);
			return outFileStream.isOpen();
		}

		u64						xfileinfo::sReadAllBytes(const xfilepath& filename, xbyte* buffer, u64 offset, u64 count)
		{
			u64 rc = 0;
			xfilestream fs(filename, FileMode_Open, FileAccess_Read, FileOp_Sync);
			if (fs.isOpen())
			{
				rc = fs.read(buffer, 0, count);
				fs.close();
			}
			return rc;
		}

		u64						xfileinfo::sWriteAllBytes(const xfilepath& filename, const xbyte* buffer, u64 offset, u64 count)
		{
			u64 rc = 0;
			xfilestream fs(filename, FileMode_Open, FileAccess_Write, FileOp_Sync);
			if (fs.isOpen())
			{
				fs.setLength(0);
				rc = fs.write(buffer, 0, count);
				fs.close();
			}
			return rc;
		}

		bool					xfileinfo::sSetTime(const xfilepath& filename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);
			if (device!=NULL)
			{
				return device->setFileTime(systemFile.c_str(), creationTime, lastAccessTime, lastWriteTime);
			}
			return false;
		}

		bool					xfileinfo::sGetTime(const xfilepath& filename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			if (device!=NULL && device->getFileTime(systemFile.c_str(), outCreationTime, outLastAccessTime, outLastWriteTime))
				return true;

			outCreationTime = xdatetime::sMinValue;
			outLastAccessTime = xdatetime::sMinValue;
			outLastWriteTime = xdatetime::sMinValue;
			return false;
		}

		bool					xfileinfo::sSetCreationTime(const xfilepath& filename, const xdatetime& creationTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);
			if (device!=NULL)
			{
				xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
				if (device->getFileTime(systemFile.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
					return device->setFileTime(systemFile.c_str(), creationTime, _lastAccessTime, _lastWriteTime);
			}
			return false;
		}

		bool					xfileinfo::sGetCreationTime(const xfilepath& filename, xdatetime& outCreationTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			xdatetime lastAccess, lastWrite;
			if (device!=NULL && device->getFileTime(systemFile.c_str(), outCreationTime, lastAccess, lastWrite))
				return true;
			outCreationTime = xdatetime::sMinValue;
			return false;
		}

		bool					xfileinfo::sSetLastAccessTime(const xfilepath& filename, const xdatetime& lastAccessTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);
			if (device!=NULL)
			{
				xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
				if (device->getFileTime(systemFile.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
					return device->setFileTime(systemFile.c_str(), _creationTime, lastAccessTime, _lastWriteTime);
			}
			return false;
		}

		bool					xfileinfo::sGetLastAccessTime(const xfilepath& filename, xdatetime& outLastAccessTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			xdatetime creation, lastWrite;
			if (device!=NULL && device->getFileTime(systemFile.c_str(), creation, outLastAccessTime, lastWrite))
				return true;
			outLastAccessTime = xdatetime::sMinValue;
			return false;
		}

		bool					xfileinfo::sSetLastWriteTime(const xfilepath& filename, const xdatetime& lastWriteTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			if (device!=NULL)
			{
				xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
				if (device->getFileTime(systemFile.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
					return device->setFileTime(systemFile.c_str(), _creationTime, _lastAccessTime, lastWriteTime);
			}
			return false;
		}

		bool					xfileinfo::sGetLastWriteTime(const xfilepath& filename, xdatetime& outLastWriteTime)
		{
			char systemDirBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring systemFile(systemDirBuffer, sizeof(systemDirBuffer));
			xfiledevice* device = filename.getSystem(systemFile);

			xdatetime creation, lastAccess;
			if (device!=NULL && device->getFileTime(systemFile.c_str(), creation, lastAccess, outLastWriteTime))
				return true;
			outLastWriteTime = xdatetime::sMinValue;
			return false;
		}


		bool					xfileinfo::sCopy(const xfilepath& filename, const xfilepath& toFilename, bool overwrite)
		{
			char srcSystemFileBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring srcSystemFile(srcSystemFileBuffer, sizeof(srcSystemFileBuffer));
			xfiledevice* src_device = filename.getSystem(srcSystemFile);
			char dstSystemFileBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring dstSystemFile(dstSystemFileBuffer, sizeof(dstSystemFileBuffer));
			xfiledevice* dst_device = toFilename.getSystem(dstSystemFile);

			if (src_device!=NULL && dst_device!=NULL)
				return src_device->copyFile(srcSystemFile.c_str(), dstSystemFile.c_str(), overwrite);

			return false;
		}

		bool					xfileinfo::sMove(const xfilepath& filename, const xfilepath& toFilename)
		{
			char srcSystemFileBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring srcSystemFile(srcSystemFileBuffer, sizeof(srcSystemFileBuffer));
			xfiledevice* src_device = filename.getSystem(srcSystemFile);
			char dstSystemFileBuffer[xfilepath::XFILEPATH_BUFFER_SIZE];
			xcstring dstSystemFile(dstSystemFileBuffer, sizeof(dstSystemFileBuffer));
			xfiledevice* dst_device = toFilename.getSystem(dstSystemFile);

			if (src_device!=NULL && dst_device!=NULL)
				return src_device->moveFile(srcSystemFile.c_str(), dstSystemFile.c_str());

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