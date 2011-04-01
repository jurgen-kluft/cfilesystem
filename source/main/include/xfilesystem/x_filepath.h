#ifndef __X_FILESYSTEM_FILEPATH_H__
#define __X_FILESYSTEM_FILEPATH_H__
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
		class xdirpath;
		class xfiledevice;

		//==============================================================================
		// xfilepath: 
		//		- Relative:		Folder\Filename.Extension
		//		- Absolute:		Device:\Folder\Folder\Filename.Extension
		//==============================================================================
		class xfilepath
		{
		public:
			enum ESettings { XFILE_MAX_PATH = 256 };

		private:
			char					mStringBuffer[XFILE_MAX_PATH];
			xcstring				mString;

		public:
									xfilepath();
									xfilepath(const char* str);
									xfilepath(const xfilepath& filepath);
			explicit				xfilepath(const xdirpath& dir, const xfilepath& filename);
									~xfilepath();

			void					clear();

			s32						length() const;
			static s32				sMaxLength()			{ return XFILE_MAX_PATH-2; }
			s32						maxLength() const;
			bool					empty() const;
			bool					isRooted() const;

			void					relative(xfilepath&) const;
			void					makeRelative();
			
			void					onlyFilename();
			xfilepath				getFilename();

			void					up();
			void					down(const char* subDir);

			void					getName(xcstring& outName) const;
			void					getExtension(xcstring& outExtension) const;
			xfiledevice*			getSystem(xcstring& outSystemFilePath) const;
			void					getDirPath(xdirpath& outDirPath) const;
			bool					getRoot(xdirpath& outRootDirPath) const;
			bool					getParent(xdirpath& outParentDirPath) const;
			void					getSubDir(const char* subDir, xdirpath& outSubDirPath) const;

			void					setDeviceName(const char* deviceName);
			void					getDeviceName(xcstring& outDeviceName) const;
			void					setDevicePart(const char* devicePart);
			void					getDevicePart(xcstring& outDevicePart) const;
			
			xfilepath&				operator =  (const char*);
			xfilepath&				operator =  (const xfilepath&);
			xfilepath&				operator += (const char*);
			xfilepath&				operator += (const xfilepath&);

			bool					operator == (const xfilepath&) const;
			bool					operator != (const xfilepath&) const;

			char					operator [] (s32 index) const;

			const char*				c_str() const;

		private:
			void					fixSlashes();														///< Fix slashes, replace '/' with '\'. For Unix, replace '\' with '/'.
		};

		inline xfilepath	operator + (const xdirpath& dir, const xfilepath& filename)	{ return xfilepath(dir, filename); }

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_FILEPATH_H__
//==============================================================================
#endif