#ifndef __X_FILESYSTEM_FILE_INFO_H__
#define __X_FILESYSTEM_FILE_INFO_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_attributes.h"

//==============================================================================
namespace xcore
{
	// Forward declares
	class xdatetime;

	// Forward declares
	class xdirinfo;


	class xfileinfo
	{
		xfilesystem*			mParent;
		xfilepath				mFilePath;
		bool					mFileExists;
		xfiletimes				mFileTimes;
		xfileattrs				mFileAttributes;
		
	public:
								xfileinfo();
								xfileinfo(const xfileinfo& fileinfo);
								xfileinfo(const xfilepath& filename);

		u64						getLength() const;
		void					setLength(u64 length);

		bool					isValid() const;
		bool					isRooted() const;
		
		xattributes				getAttrs() const;
		xfiletimes				getTimes() const;

		bool					setAttrs(xattributes attrs);
		bool					setTimes(xfiletimes times);

		bool					exists() const;
		bool					remove();
		void					refresh();

		bool					create();
	
		bool					copy(const xfilepath& toFilename, bool overwrite);
		bool					move(const xfilepath& toFilename, bool overwrite);

		xfileinfo&				operator = (const xfileinfo&);
		xfileinfo&				operator = (const xfilepath&);

		bool					operator == (const xfileinfo&) const;
		bool					operator != (const xfileinfo&) const;

		///< Static functions
		static bool				sExists(const xfilepath& filename);
		static bool				sCreate(const xfilepath& filename, xfilestream& outFileStream);
		static bool				sDelete(const xfilepath& filename);

		static u64				sGetLength(const xfilepath& filename);
		static void				sSetLength(const xfilepath& filename, u64 length);

		static bool				sIsArchive(const xfilepath& filename);
		static bool				sIsReadOnly(const xfilepath& filename);
		static bool				sIsHidden(const xfilepath& filename);
		static bool				sIsSystem(const xfilepath& filename);

		static bool				sOpen(const xfilepath& filename, xfilestream& outFileStream);
		static bool				sOpenRead(const xfilepath& filename, xfilestream& outFileStream);
		static bool				sOpenWrite(const xfilepath& filename, xfilestream& outFileStream);
		
		///< Opens a binary file, reads the contents of the file into a byte buffer, and then closes the file.
		static u64				sReadAllBytes(const xfilepath& filename, xbyte* buffer, u64 offset, u64 count);
		///< Creates a new file, writes the specified byte buffer to the file, and then closes the file. If the target file already exists, it is overwritten.
		static u64				sWriteAllBytes(const xfilepath& filename, const xbyte* buffer, u64 offset, u64 count);

		static bool				sSetTime(const xfilepath& filename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime);
		static bool				sGetTime(const xfilepath& filename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime);
		static bool				sSetCreationTime(const xfilepath& filename, const xdatetime& creationTime);
		static bool				sGetCreationTime(const xfilepath& filename, xdatetime& outCreationTime);
		static bool				sSetLastAccessTime(const xfilepath& filename, const xdatetime& lastAccessTime);
		static bool				sGetLastAccessTime(const xfilepath& filename, xdatetime& outLastAccessTime);
		static bool				sSetLastWriteTime(const xfilepath& filename, const xdatetime& lastWriteTime);
		static bool				sGetLastWriteTime(const xfilepath& filename, xdatetime& outLastWriteTime);

		///< Copies an existing file to a new file. Overwriting a file of the same name is allowed.
		static bool				sCopy(const xfilepath& sourceFilename, const xfilepath& destFilename, bool overwrite);
		///< Moves a specified file to a new location, providing the option to specify a new file name.
		static bool				sMove(const xfilepath& sourceFilename, const xfilepath& destFilename);
	};

};

#endif
