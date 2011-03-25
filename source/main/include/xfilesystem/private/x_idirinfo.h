#ifndef __X_FILESYSTEM_DIR_INFO_INTERFACE_H__
#define __X_FILESYSTEM_DIR_INFO_INTERFACE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

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

		template<typename _Arg>
		struct enumerate_delegate;

		class xidirinfo
		{
		public:
			virtual							~xidirinfo();

			virtual const xdirpath&			getFullName() const = 0;
			virtual bool					getName(char* str, s32 strMaxLength) const = 0;

			virtual bool					isValid() const = 0;
			virtual bool					isAbsolute() const = 0;

			virtual bool					exists() const = 0;
			virtual bool					create() = 0;
			virtual bool					remove() = 0;
			virtual void					refresh() = 0;

			virtual bool					copy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, bool overwrite) = 0;
			virtual bool					move(const xdirpath& sourceDirectory, const xdirpath& destDirectory) = 0;

			virtual void					enumerateFiles(enumerate_delegate<xfileinfo>& enumerator, bool searchSubDirectories) = 0;
			virtual void					enumerateDirs(enumerate_delegate<xdirinfo>& enumerator, bool searchSubDirectories) = 0;

			virtual const xdevicealias*		getAlias() const = 0;
			virtual void					getRoot(xdirinfo& outInfo) const = 0;
			virtual bool					getParent(xdirinfo& outInfo) const = 0;
			virtual bool					getSubdir(const char* subDir, xdirinfo& outInfo) const = 0;

			virtual void					getCreationTime  (xdatetime&) const = 0;
			virtual void					getLastAccessTime(xdatetime&) const = 0;
			virtual void					getLastWriteTime (xdatetime&) const = 0;
			virtual void					setCreationTime  (const xdatetime&) const = 0;
			virtual void					setLastAccessTime(const xdatetime&) const = 0;
			virtual void					setLastWriteTime (const xdatetime&) const = 0;
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
// END __X_FILESYSTEM_DIR_INFO_INTERFACE_H__
//==============================================================================
#endif