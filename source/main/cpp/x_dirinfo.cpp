#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string.h"
#include "xbase\x_string_std.h"
#include "xtime\x_time.h"

#include "xfilesystem\x_dirinfo.h"

#include "xfilesystem\x_devicealias.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_idirinfo.h"


//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xidirinfo_imp : public xidirinfo
		{
			xdirpath						mDirPath;

		public:
											xidirinfo_imp(const xdirpath& dirpath)
												: mDirPath(dirpath)								{ }
			virtual							~xidirinfo_imp()									{ }

			virtual const xdirpath&			getFullName() const
			{
				return mDirPath; 
			}
			
			virtual bool					getName(char* str, s32 strMaxLength) const
			{
				return mDirPath.getName(str, strMaxLength);
			}

			virtual bool					isValid() const
			{
				char deviceStrBuffer[64];
				mDirPath.getDeviceName(deviceStrBuffer, sizeof(deviceStrBuffer)-1);
				const s32 deviceStrLen = x_strlen(deviceStrBuffer);
				if (deviceStrLen == 0)
					return true;
				const xdevicealias* _alias = xdevicealias::sFind(deviceStrBuffer);				
				return (_alias != NULL);
			}

			virtual bool					isAbsolute() const
			{
				char deviceStrBuffer[64];
				mDirPath.getDeviceName(deviceStrBuffer, sizeof(deviceStrBuffer)-1);
				const s32 deviceStrLen = x_strlen(deviceStrBuffer);
				return (deviceStrLen != 0);
			}

			virtual bool					exists() const
			{
				const xdevicealias* _alias = getAlias();
				return _alias!=NULL && _alias->device()->hasDir(mDirPath.c_str());
			}

			virtual bool					create()
			{
				const xdevicealias* _alias = getAlias();
				u32 nFileHandle;
				if (_alias!=NULL && _alias->device()->createFile(mDirPath.c_str(), true, true, nFileHandle))
				{
					return _alias->device()->closeFile(nFileHandle);
				}
				return false;
			}

			virtual bool					remove()
			{
				const xdevicealias* _alias = getAlias();
				return _alias!=NULL && _alias->device()->deleteFile(mDirPath.c_str());
			}

			virtual void					refresh()
			{
				
			}

			virtual bool					copy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, bool overwrite)
			{
			}

			virtual bool					move(const xdirpath& sourceDirectory, const xdirpath& destDirectory)
			{
			}

			virtual void					enumerateFiles(enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories)
			{
			}

			virtual void					enumerateDirs(enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories)
			{
			}

			virtual const xdevicealias*		getAlias() const
			{
				char deviceStrBuffer[64];
				mDirPath.getDeviceName(deviceStrBuffer, sizeof(deviceStrBuffer)-1);
				const s32 deviceStrLen = x_strlen(deviceStrBuffer);
				const xdevicealias* alias = xdevicealias::sFind( (deviceStrLen == 0) ? "currdir" : deviceStrBuffer);
				return alias;
			}

			virtual void					getRoot(xdirinfo& outInfo) const
			{
			}

			virtual bool					getParent(xdirinfo& outInfo) const
			{
			}

			virtual bool					getSubdir(const char* subDir, xdirinfo& outInfo) const
			{
			}

			virtual void					getCreationTime  (xdatetime& creationTime) const
			{
				const xdevicealias* _alias = getAlias();
				xdatetime lastAccess, lastWrite;
				if (_alias==NULL || _alias->device()->getDirTime(mDirPath.c_str(), creationTime, lastAccess, lastWrite)==false)
					creationTime = xdatetime::sMinValue;
			}

			virtual void					getLastAccessTime(xdatetime& lastAccessTime) const
			{
				const xdevicealias* _alias = getAlias();
				xdatetime creation, lastWrite;
				if (_alias==NULL || _alias->device()->getDirTime(mDirPath.c_str(), creation, lastAccessTime, lastWrite)==false)
					lastAccessTime = xdatetime::sMinValue;
			}

			virtual void					getLastWriteTime (xdatetime& lastWriteTime) const
			{
				const xdevicealias* _alias = getAlias();
				xdatetime creation, lastAccess;
				if (_alias==NULL || _alias->device()->getDirTime(mDirPath.c_str(), creation, lastAccess, lastWriteTime)==false)
					lastWriteTime = xdatetime::sMinValue;
			}

			virtual void					setLastAccessTime(const xdatetime&) const
			{
			}

			virtual void					setLastWriteTime (const xdatetime&) const
			{
			}
		};


		static xidirinfo*	sCreateDirInfoImp()
		{

		}

		xdirinfo::xdirinfo()
		{
		}

		xdirinfo::xdirinfo(const xdirinfo& dirinfo)
		{
		}

		xdirinfo::xdirinfo(const char* dir)
		{
		}

		xdirinfo::xdirinfo(const xdirpath& dir)
		{
		}

		const xdirpath&			xdirinfo::getFullName() const
		{
		}

		xbool					xdirinfo::getName(char* str, s32 strMaxLength) const
		{
		}


		xbool					xdirinfo::isValid() const
		{
		}

		xbool					xdirinfo::isAbsolute() const
		{
		}


		xbool					xdirinfo::exists() const
		{
		}

		void					xdirinfo::create()
		{
		}

		void					xdirinfo::remove()
		{
		}

		void					xdirinfo::refresh()
		{
		}


		void					xdirinfo::copy(xdirpath& sourceDirectory, xdirpath& destDirectory, xbool overwrite)
		{
		}

		void					xdirinfo::move(xdirpath& sourceDirectory, xdirpath& destDirectory)
		{
		}


		void					xdirinfo::enumerateFiles(enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories)
		{
		}

		void					xdirinfo::enumerateDirs(enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories)
		{
		}


		const xdevicealias*		xdirinfo::getAlias() const
		{
		}

		xdirinfo				xdirinfo::getRoot() const
		{
		}

		xdirinfo				xdirinfo::getParent() const
		{
		}

		xdirinfo				xdirinfo::getSubdir(const char*) const
		{
		}


		void					xdirinfo::getCreationTime  (xdatetime&) const
		{
		}

		void					xdirinfo::getLastAccessTime(xdatetime&) const
		{
		}

		void					xdirinfo::getLastWriteTime (xdatetime&) const
		{
		}

		void					xdirinfo::setCreationTime  (const xdatetime&) const
		{
		}

		void					xdirinfo::setLastAccessTime(const xdatetime&) const
		{
		}

		void					xdirinfo::setLastWriteTime (const xdatetime&) const
		{
		}


		void					xdirinfo::operator = (const xdirinfo&) const
		{
		}


		bool					xdirinfo::operator == (const xdirinfo&) const
		{
		}

		bool					xdirinfo::operator != (const xdirinfo&) const
		{
		}


		///< Static functions
		void					xdirinfo::xdirinfo::sCreate(xdirpath& directory)
		{
		}

		void					xdirinfo::xdirinfo::sDelete(xdirpath& directory)
		{
		}

		bool					xdirinfo::xdirinfo::sExists(xdirpath& directory)
		{
		}


		void					xdirinfo::sEnumerateFiles(xdirpath& directory, enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories)
		{
		}

		void					xdirinfo::sEnumerateDirs(xdirpath& directory, enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories)
		{
		}

		void					xdirinfo::sSetCreationTime(xdirpath& directory, const xdatetime& creationTime)
		{
		}

		void					xdirinfo::sGetCreationTime(xdirpath& directory, xdatetime& outCreationTime)
		{
		}

		void					xdirinfo::sSetLastAccessTime(xdirpath& directory, const xdatetime& lastAccessTime)
		{
		}

		void					xdirinfo::sGetLastAccessTime(xdirpath& directory, xdatetime& outLastAccessTime)
		{
		}

		void					xdirinfo::sSetLastWriteTime(xdirpath& directory, const xdatetime& lastWriteTime)
		{
		}

		void					xdirinfo::sGetLastWriteTime(xdirpath& directory, xdatetime& outLastWriteTime)
		{
		}


		void					xdirinfo::sCopy(xdirpath& sourceDirectory, xdirpath& destDirectory, xbool overwrite)
		{
		}

		void					xdirinfo::sMove(xdirpath& sourceDirectory, xdirpath& destDirectory)
		{
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
