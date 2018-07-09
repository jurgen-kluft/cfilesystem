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
		class xpath;
		class xfilepath;
		class xfiledevice;
		class xdevicealias;

		//==============================================================================
		// xdirpath: 
		//		- Relative:		FolderA\FolderB\
		//		- Absolute:		Device:\FolderA\FolderB\
		//==============================================================================
		class xdirpath
		{
		public:
			enum ESettings { MAXPATH_SIZE = 256, MAX = MAXPATH_SIZE-2 };

		protected:
			friend class xfilepath;
			xucharz<MAXPATH_SIZE>	mString;

		public:
									xdirpath();
									xdirpath(const char* str);
									xdirpath(const xdirpath& dir);
									~xdirpath();

			void					clear();

			s32						getLength() const;
			s32						getMaxLength() const;
			bool					isEmpty() const;

			void					enumLevels(enumerate_delegate<xucharz<256>>& folder_enumerator, bool right_to_left = false) const;
			s32						getLevels() const;
			s32						getLevelOf(const xcuchars& folderName) const;
			s32						getLevelOf(const xdirpath& parent) const;

			bool					isRoot() const;
			bool					isRooted() const;
			bool					isSubDirOf(const xdirpath&) const;
			
			void					relative(xdirpath& outRelative) const;
			void					makeRelative();
			void					makeRelativeTo(const xdirpath& parent);

			void					up();
			void					down(const xcuchars& subDir);
			void					split(s32 cnt, xdirpath& parent, xdirpath& subDir) const;	///< e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\; sub=="sub\\folder\\";

			bool					getName(xuchars& outName) const;
			bool					hasName(const xcuchars& inName) const;
			const xdevicealias*		getAlias() const;
			xfiledevice*			getDevice() const;
			xfiledevice*			getSystem(xuchars& outSystemDirPath) const;
			bool					getRoot(xdirpath& outRootDirPath) const;
			bool					getParent(xdirpath& outParentDirPath) const;
			bool					getSubDir(const xcuchars& subDir, xdirpath& outSubDirPath) const;

			void					setDeviceName(const xcuchars& deviceName);
			bool					getDeviceName(xuchars& outDeviceName) const;
			void					setDevicePart(const xcuchars& devicePart);
			bool					getDevicePart(xuchars& outDevicePart) const;
			
			xdirpath&				operator =  (const xdirpath&);

			xdirpath&				operator =  (const xcuchars&);
			xdirpath&				operator += (const xcuchars&);

			bool					operator == (const xcuchars&) const;
			bool					operator != (const xcuchars&) const;

			bool					operator == (const xdirpath& rhs) const;
			bool					operator != (const xdirpath& rhs) const;

			const xcuchars&			cchars() const;

		private:
			void					setOrReplaceDeviceName(xuchars& ioStr, xcuchars const& inDeviceName) const;
			void					setOrReplaceDevicePart(xuchars& ioStr, xcuchars const& inDeviceName) const;

			void					fixSlashes();									///< Fix slashes, replace '/' with '\'. For Unix, replace '\' with '/'.
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
// END __X_FILESYSTEM_DIRPATH_H__
//==============================================================================
#endif