#ifndef __X_FILESYSTEM_DIR_INFO_H__
#define __X_FILESYSTEM_DIR_INFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
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
		class xdirinfo;
		class xdirinfo_imp;

		class xdirinfo
		{
			xdirinfo_imp*			mImplementation;

		public:
									xdirinfo();
									xdirinfo(const xdirinfo& dirinfo);
									xdirinfo(const char* dir);
									xdirinfo(const xdirpath& dir);

			const xdirpath&			getFullName() const;
			xbool					getName(char* str, s32 strMaxLength) const;

			xbool					isValid() const;
			xbool					isAbsolute() const;

			xbool					exists() const;
			void					create();
			void					remove();
			void					refresh();

			void					copy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, xbool overwrite=xFALSE);
			void					move(const xdirpath& sourceDirectory, const xdirpath& destDirectory);

			void					enumerateFiles(enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories);
			void					enumerateDirs(enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories);

			const xdevicealias*		getAlias() const;
			xdirinfo				getRoot() const;
			xdirinfo				getParent() const;
			xdirinfo				getSubdir(const char*) const;

			void					getCreationTime  (xdatetime&) const;
			void					getLastAccessTime(xdatetime&) const;
			void					getLastWriteTime (xdatetime&) const;
			void					setCreationTime  (const xdatetime&) const;
			void					setLastAccessTime(const xdatetime&) const;
			void					setLastWriteTime (const xdatetime&) const;

			void					operator = (const xdirinfo&) const;

			bool					operator == (const xdirinfo&) const;
			bool					operator != (const xdirinfo&) const;

			///< Static functions
			static void				sCreate(const xdirpath& directory);
			static void				sDelete(const xdirpath& directory);
			static bool				sExists(const xdirpath& directory);

			static void				sEnumerateFiles(const xdirpath& directory, enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories);
			static void				sEnumerateDirs(const xdirpath& directory, enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories);

			static void				sGetCreationTime(const xdirpath& directory, xdatetime& outCreationTime);
			static void				sSetLastAccessTime(const xdirpath& directory, const xdatetime& lastAccessTime);
			static void				sGetLastAccessTime(const xdirpath& directory, xdatetime& outLastAccessTime);
			static void				sSetLastWriteTime(const xdirpath& directory, const xdatetime& lastWriteTime);
			static void				sGetLastWriteTime(const xdirpath& directory, xdatetime& outLastWriteTime);

			///< Copies an existing directory to a new directory. Overwriting a directory of the same name is allowed.
			static void				sCopy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, xbool overwrite=xFALSE);
			///< Moves a specified directory to a new location, providing the option to specify a new directory name.
			static void				sMove(const xdirpath& sourceDirectory, const xdirpath& destDirectory);
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