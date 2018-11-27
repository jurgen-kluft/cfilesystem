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
	//==============================================================================
	class xfilepath
	{
	protected:
		friend class xdirpath;
		friend class xfilesystem;

		xfilesystem*			mParent;
		xstring					mString;

								xfilepath(xstring const& str);
	public:
								xfilepath();
								xfilepath(const xfilepath& filepath);
		explicit				xfilepath(const xdirpath& dir, const xfilepath& filename);
								~xfilepath();

		void					clear();

		bool					isEmpty() const;
		bool					isRooted() const;

		void					makeRelative(xdirpath&);
		void					makeRelative(xdirpath&, xfilepath&) const;
		
		void					getFull(xstring& ) const;
		void					getFilepath(xstring& ) const;
		void					getFilename(xstring&) const;
		void					getFilenameWithoutExtension(xstring&) const;
		void					getExtension(xstring&) const;
		void					getDirname(xdirpath&) const;

		xfilepath&				operator =  (const xfilepath&);
		bool					operator == (const xfilepath&) const;
		bool					operator != (const xfilepath&) const;
	};

	inline xfilepath	operator + (const xdirpath& dir, const xfilepath& filename)	{ return xfilepath(dir, filename); }

};

#endif