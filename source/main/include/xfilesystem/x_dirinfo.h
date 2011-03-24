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

		template<typename _Arg>
		struct enumerate_delegate
		{
			///< Return false to terminate the breadth first traversal
			virtual bool operator () (s32 depth) = 0;
			virtual void operator () (s32 depth, const _Arg& inf) = 0;
		};

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

			void					copy(xdirpath& sourceDirectory, xdirpath& destDirectory, xbool overwrite=xFALSE);
			void					move(xdirpath& sourceDirectory, xdirpath& destDirectory);

			void					enumerateFiles(enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories=false);
			void					enumerateDirs(enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories=false);

			const xdevicealias*			getAlias() const;
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
			static void				sCreate(xdirpath& directory);
			static void				sDelete(xdirpath& directory);
			static bool				sExists(xdirpath& directory);

			static void				sEnumerateFiles(xdirpath& directory, enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories=false);
			static void				sEnumerateDirs(xdirpath& directory, enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories=false);

			static void				sSetCreationTime(xdirpath& directory, const xdatetime& creationTime);
			static void				sGetCreationTime(xdirpath& directory, xdatetime& outCreationTime);
			static void				sSetLastAccessTime(xdirpath& directory, const xdatetime& lastAccessTime);
			static void				sGetLastAccessTime(xdirpath& directory, xdatetime& outLastAccessTime);
			static void				sSetLastWriteTime(xdirpath& directory, const xdatetime& lastWriteTime);
			static void				sGetLastWriteTime(xdirpath& directory, xdatetime& outLastWriteTime);

			///< Copies an existing directory to a new directory. Overwriting a directory of the same name is allowed.
			static void				sCopy(xdirpath& sourceDirectory, xdirpath& destDirectory, xbool overwrite=xFALSE);
			///< Moves a specified directory to a new location, providing the option to specify a new directory name.
			static void				sMove(xdirpath& sourceDirectory, xdirpath& destDirectory);
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