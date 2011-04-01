#ifndef __X_FILESYSTEM_DIRPATH_H__
#define __X_FILESYSTEM_DIRPATH_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xpath;
		class xfilepath;
		class xfiledevice;

		//==============================================================================
		// xdirpath: 
		//		- Relative:		FolderA\FolderB\
		//		- Absolute:		Device:\FolderA\FolderB\
		//==============================================================================
		class xdirpath
		{
		public:
			enum ESettings { XDIR_MAX_PATH = 256 };

		protected:
			friend class xfilepath;
			char					mStringBuffer[XDIR_MAX_PATH];
			xcstring				mString;

		public:
									xdirpath();
									xdirpath(const char* str);
									xdirpath(const xdirpath& dir);
									~xdirpath();

			void					clear();

			s32						getLength() const;
			static s32				sMaxLength()			{ return XDIR_MAX_PATH-2; }
			s32						getMaxLength() const;
			bool					isEmpty() const;

			s32						getLevels() const;

			bool					isRoot() const;
			bool					isRooted() const;
			
			void					relative(xdirpath& outRelative) const;
			void					makeRelative();

			void					up();
			void					down(const char* subDir);
			void					split(s32 cnt, xdirpath& parent, xdirpath& subDir) const;	///< e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\; sub=="sub\\folder\\";

			bool					getName(xcstring& outName) const;
			bool					hasName(const char* inName) const;
			xfiledevice*			getSystem(xcstring& outSystemDirPath) const;
			bool					getRoot(xdirpath& outRootDirPath) const;
			bool					getParent(xdirpath& outParentDirPath) const;
			bool					getSubDir(const char* subDir, xdirpath& outSubDirPath) const;

			void					setDeviceName(const char* deviceName);
			void					getDeviceName(xcstring& outDeviceName) const;
			void					setDevicePart(const char* devicePart);
			void					getDevicePart(xcstring& outDevicePart) const;
			
			xdirpath&				operator =  (const xdirpath&);

			xdirpath&				operator =  (const char*);
			xdirpath&				operator += (const char*);

			bool					operator == (const xdirpath& rhs) const;
			bool					operator != (const xdirpath& rhs) const;

			char					operator [] (s32 index) const;

			const char*				c_str() const;

		private:
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