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
#include "xbase/x_chars.h"

#include "xfilesystem/x_enumerator.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xfilepath;
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
			friend class xfilesystem;
			
			xfilesystem*			mParent;
			xstring					mString;

									xdirpath(xfilesystem* fs);
									xdirpath(xfilesystem* fs, xstring const& str);
		public:
									xdirpath(const xdirpath& dir);
									~xdirpath();

			void					clear();

			s32						getLength() const;
			s32						getMaxLength() const;
			bool					isEmpty() const;

			s32						getLevels() const;
			bool					getLevel(s32 level, xstring& name) const;
			s32						getLevelOf(const xstring& folderName) const;
			s32						getLevelOf(const xdirpath& parent) const;

			bool					isRoot() const;
			bool					isRooted() const;
			bool					isSubDirOf(const xdirpath&) const;
			
			void					relative(xdirpath& outRelative) const;
			void					makeRelative();
			void					makeRelativeTo(const xdirpath& parent);

			void					up();
			void					down(const xstring& subDir);
			void					split(s32 level, xdirpath& parent, xdirpath& subDir) const;	///< e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\; sub=="sub\\folder\\";

			void					getFull(xstring& full) const;
			bool					getName(xstring& outName) const;
			bool					hasName(const xstring& inName) const;
			bool					getRoot(xdirpath& outRootDirPath) const;
			bool					getParent(xdirpath& outParentDirPath) const;
			bool					getSubDir(const xstring& subDir, xdirpath& outSubDirPath) const;

			void					setDeviceName(const xstring& deviceName);
			bool					getDeviceName(xstring& outDeviceName) const;
			void					setDevicePart(const xstring& devicePart);
			bool					getDevicePart(xstring& outDevicePart) const;

			xdirpath&				operator =  (const xfilepath&);
			xdirpath&				operator =  (const xdirpath&);
			xfilepath&				operator += (const xfilepath&);

			bool					operator == (const xdirpath& rhs) const;
			bool					operator != (const xdirpath& rhs) const;
		};
	};
};

#endif	// __X_FILESYSTEM_DIRPATH_H__