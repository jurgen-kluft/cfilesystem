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
			enum ESettings { XDIRPATH_BUFFER_SIZE = 256, XDIRPATH_MAX = XDIRPATH_BUFFER_SIZE-2 };

		protected:
			friend class xfilepath;
			char					mStringBuffer[XDIRPATH_BUFFER_SIZE];
			xcstring				mString;
			char					mStringBufferForDevice[XDIRPATH_BUFFER_SIZE];
			xcstring				mStringForDevice;

		public:
									xdirpath();
									xdirpath(const char* str);
									xdirpath(const xdirpath& dir);
									~xdirpath();

			void					clear();

			s32						getLength() const;
			s32						getMaxLength() const;
			bool					isEmpty() const;

			void					enumLevels(enumerate_delegate<char>& folder_enumerator, bool right_to_left = false) const;
			s32						getLevels() const;
			s32						getLevelOf(const char* folderName, s32 numChars=-1) const;
			s32						getLevelOf(const xdirpath& parent) const;

			bool					isRoot() const;
			bool					isRooted() const;
			bool					isSubDirOf(const xdirpath&) const;
			
			void					relative(xdirpath& outRelative) const;
			void					makeRelative();
			void					makeRelativeTo(const xdirpath& parent);

			void					up();
			void					down(const char* subDir);
			void					split(s32 cnt, xdirpath& parent, xdirpath& subDir) const;	///< e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\; sub=="sub\\folder\\";

			bool					getName(xcstring& outName) const;
			bool					hasName(const char* inName) const;
			const xdevicealias*		getAlias() const;
			xfiledevice*			getDevice() const;
			xfiledevice*			getSystem(xcstring& outSystemDirPath) const;
			bool					getRoot(xdirpath& outRootDirPath) const;
			bool					getParent(xdirpath& outParentDirPath) const;
			bool					getSubDir(const char* subDir, xdirpath& outSubDirPath) const;

			void					setDeviceName(const char* deviceName);
			bool					getDeviceName(xcstring& outDeviceName) const;
			void					setDevicePart(const char* devicePart);
			bool					getDevicePart(xcstring& outDevicePart) const;
			
			xdirpath&				operator =  (const xdirpath&);

			xdirpath&				operator =  (const char*);
			xdirpath&				operator += (const char*);

			bool					operator == (const char* rhs) const;
			bool					operator != (const char* rhs) const;

			bool					operator == (const xdirpath& rhs) const;
			bool					operator != (const xdirpath& rhs) const;

			char					operator [] (s32 index) const;

			const char*				c_str() const;

			const char*               c_str_device() const;
#ifdef TARGET_PS3
			bool					makeRelativeForPS3();	
#endif
		private:
			void					setOrReplaceDeviceName(xcstring&, const char*) const;
			void					setOrReplaceDevicePart(xcstring&, const char*) const;

			void					fixSlashes();									///< Fix slashes, replace '/' with '\'. For Unix, replace '\' with '/'.
			void					fixSlashesForDevice();
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