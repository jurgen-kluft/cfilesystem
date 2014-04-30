#ifndef __X_FILESYSTEM_DIR_INFO_H__
#define __X_FILESYSTEM_DIR_INFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_debug.h"

#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\x_enumerator.h"

//==============================================================================
namespace xcore
{
	// Forward declares
	class xdatetime;

	namespace xfilesystem
	{
		// Forward declares
		class xdevicealias;
		class xfileinfo;

		class xdirinfo
		{
			xdirpath				mDirPath;

		public:
									xdirinfo();
									xdirinfo(const xdirinfo& dirinfo);
			explicit				xdirinfo(const char* dir);
			explicit				xdirinfo(const xdirpath& dir);

			const xdirpath&			getFullName() const;
			bool					getName(xcstring& outName) const;

			bool					isValid() const;
			bool					isRoot() const;
			bool					isRooted() const;

			bool					exists() const;
			bool					create();
			bool					remove();
			void					refresh();

			void					copy(const xdirpath& toDirectory, xbool overwrite=xFALSE);
			void					move(const xdirpath& toDirectory);

			void					enumerateFiles(enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories);
			void					enumerateDirs(enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories);

			const xdevicealias*		getAlias() const;
			bool					getRoot(xdirinfo& outRootDirInfo) const;
			bool					getParent(xdirinfo& outParentDirInfo) const;
			bool					getSubdir(const char* subDir, xdirinfo& outSubDirInfo) const;

			bool					getCreationTime  (xdatetime&) const;
			bool					getLastAccessTime(xdatetime&) const;
			bool					getLastWriteTime (xdatetime&) const;
			bool					setCreationTime  (const xdatetime&) const;
			bool					setLastAccessTime(const xdatetime&) const;
			bool					setLastWriteTime (const xdatetime&) const;
				
			xdirinfo&				operator = (const char*);
			xdirinfo&				operator = (const xdirinfo&);
			xdirinfo&				operator = (const xdirpath&);

			bool					operator == (const char*) const;
			bool					operator != (const char*) const;

			bool					operator == (const xdirpath&) const;
			bool					operator != (const xdirpath&) const;

			bool					operator == (const xdirinfo&) const;
			bool					operator != (const xdirinfo&) const;

			///< Static functions
			static bool				sIsValid(const xdirpath& directory);

			static bool				sExists(const xdirpath& directory);
			static bool				sCreate(const xdirpath& directory);
			static bool				sDelete(const xdirpath& directory);

			static void				sEnumerate(const xdirpath& directory, enumerate_delegate<xfileinfo>& file_enumerator, enumerate_delegate<xdirinfo>& dir_enumerator, bool searchSubDirectories);
			static void				sEnumerateFiles(const xdirpath& directory, enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories);
			static void				sEnumerateDirs(const xdirpath& directory, enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories);

			static bool				sSetTime(const xdirpath& directory, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime);
			static bool				sGetTime(const xdirpath& directory, xdatetime& creationTime, xdatetime& lastAccessTime, xdatetime& lastWriteTime);

			static bool				sSetCreationTime(const xdirpath& directory, const xdatetime& lastWriteTime);
			static bool				sGetCreationTime(const xdirpath& directory, xdatetime& outCreationTime);
			static bool				sSetLastAccessTime(const xdirpath& directory, const xdatetime& lastAccessTime);
			static bool				sGetLastAccessTime(const xdirpath& directory, xdatetime& outLastAccessTime);
			static bool				sSetLastWriteTime(const xdirpath& directory, const xdatetime& lastWriteTime);
			static bool				sGetLastWriteTime(const xdirpath& directory, xdatetime& outLastWriteTime);

			///< Copies an existing directory to a new directory. Overwriting a directory of the same name is allowed.
			static bool				sCopy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, xbool overwrite=xFALSE);
			///< Moves a specified directory to a new location, providing the option to specify a new directory name.
			static bool				sMove(const xdirpath& sourceDirectory, const xdirpath& destDirectory);
		};

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_DIR_INFO_H__
//==============================================================================
#endif