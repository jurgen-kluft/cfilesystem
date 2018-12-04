#ifndef __X_FILESYSTEM_DIRPATH_H__
#define __X_FILESYSTEM_DIRPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_enumerator.h"

//==============================================================================
namespace xcore
{
	class xfilepath;
	class xfiledevice;
	class xfilesystem;

	//==============================================================================
	// xdirpath:
	//		- Relative:		FolderA\FolderB\ 
	//		- Absolute:		Device:\FolderA\FolderB\ 
	//==============================================================================
	class xdirpath
	{
	protected:
		friend class xfilepath;
		friend class xfileinfo;
		friend class xdirinfo;
		friend class xfilesystem;

		xdirpath resolve(xfiledevice*& outdevice) const;

		xdirpath(xfilesystem* fs, utf16::alloc* allocator);
		xdirpath(xfilesystem* fs, utf16::alloc* allocator, const utf16::runes& str);

	public:
		xfilesystem*  mParent;
		utf16::alloc* mAlloc;
		utf16::runes  mRunes;

	public:
		xdirpath();
		xdirpath(const xdirpath& dir);
		xdirpath(const xdirpath& rootdir, const xdirpath& subdir);
		~xdirpath();

		void clear();

		bool isEmpty() const;

		s32  getLevels() const;
		bool getLevel(s32 level, xdirpath& name) const;
		s32  getLevelOf(const xdirpath& name) const;

		bool isRoot() const;
		bool isRooted() const;
		bool isSubDirOf(const xdirpath&) const;

		void relative(xdirpath& outRelative) const;
		void makeRelative();
		void makeRelativeTo(const xdirpath& parent);
		void makeRelativeTo(const xdirpath& parent, xdirpath& sub) const;

		bool split(s32 level, xdirpath& parent,
				   xdirpath& subDir) const; ///< e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\; sub=="sub\\folder\\";

		bool getName(xfilepath& outName) const;
		bool hasName(const xfilepath& inName) const;
		bool getRoot(xdirpath& outRootDirPath) const;
		bool getParent(xdirpath& outParentDirPath) const;
		bool getSubDir(const xfilepath& subDir, xdirpath& outSubDirPath) const;

		void setRoot(const xdirpath& device);
		bool getRoot(xdirpath& outDevice) const;

		xdirpath& operator=(const xdirpath&);
		xdirpath& operator=(const xfilepath&);
		xdirpath& operator+=(const xdirpath&);
		xfilepath operator+=(const xfilepath&);

		bool operator==(const xdirpath& rhs) const;
		bool operator!=(const xdirpath& rhs) const;
	};

	xdirpath  operator+(const xdirpath&, const xdirpath&);
	xfilepath operator+(const xdirpath&, const xfilepath&);
};

#endif // __X_FILESYSTEM_DIRPATH_H__