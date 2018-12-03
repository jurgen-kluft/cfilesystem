#ifndef __X_FILESYSTEM_FILEPATH_H__
#define __X_FILESYSTEM_FILEPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

//==============================================================================
namespace xcore
{
	class xpath;
	class xdirpath;
	class xfilesystem;

	//==============================================================================
	// xfilepath:
	//		- Relative:		Folder\Filename.Extension
	//		- Absolute:		Device:\Folder\Folder\Filename.Extension
	//
	// What about making the following classes:
	// - xfilext; for file extensions, ".JPG"
	// - xfilename; for file names, "MyImage"
	// - xroot; for device part, "Device:\"
	//
	// "C:\" + "Windows\" + "DBGHELP" + ".DLL"
	// xroot + xdirpath + xfilename + xfilext
	//
	//==============================================================================
	class xfilepath
	{
	protected:
		friend class xdirpath;
		friend class xfilesystem;

		xfilesystem*  mParent;
		utf16::alloc* mAlloc;

		/*
		It seems that a utf16::runes should be sufficient here and a special allocator
		for allocating utf16 slices should be the most sufficient way of dealing with
		strings here in the filesystem.
		Filepath should always make sure that it is the sole owner of the utf16 slice and
		no sharing should be allowed outside of this class.
		*/

		xfilepath(xfilesystem* parent, utf16::alloc* allocator, utf16::runes& str);

	public:
		utf16::runes mRunes;

	public:
		xfilepath();
		xfilepath(const xfilepath& filepath);
		explicit xfilepath(const xdirpath& dir, const xfilepath& filename);
		~xfilepath();

		void clear();

		bool isEmpty() const;
		bool isRooted() const;

		void makeRelative(const xdirpath&);
		void makeRelative(xdirpath&, xfilepath&) const;

		void getFilename(xfilepath&) const;
		void getFilenameWithoutExtension(xfilepath&) const;
		void getExtension(xfilepath&) const;
		void getDirname(xdirpath&) const;

		xfilepath& operator=(const xfilepath&);
		bool	   operator==(const xfilepath&) const;
		bool	   operator!=(const xfilepath&) const;
	};

	inline xfilepath operator+(const xdirpath& dir, const xfilepath& filename)
	{
		return xfilepath(dir, filename);
	}

}; // namespace xcore

#endif