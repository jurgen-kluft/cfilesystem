#ifndef __X_FILESYSTEM_FILE_INFO_H__
#define __X_FILESYSTEM_FILE_INFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_debug.h"

#include "xfilesystem\x_filepath.h"

//==============================================================================
namespace xcore
{
	// Forward declares
	class xdatetime;

	namespace xfilesystem
	{
		// Forward declares
		class xdevicealias;
		class xdirinfo;
		class xfilestream;

		class xfileinfo
		{
			xfilepath				mFilePath;
			
		public:
									xfileinfo();
									xfileinfo(const xfileinfo& fileinfo);
									xfileinfo(const char* filename);
									xfileinfo(const xfilepath& filename);

			const xfilepath&		getFullName() const;
			void					getName(xcstring& outName) const;
			void					getExtension(xcstring& outExtension) const;
			
			u64						getLength() const;
			void					setLength(u64 length);

			bool					isValid() const;
			bool					isRooted() const;
			
			bool					isArchive() const;
			bool					isReadOnly() const;
			bool					isHidden() const;
			bool					isSystem() const;

			bool					exists() const;
			bool					create();
			bool					create(xfilestream& outFileStream);
			bool					remove();
			void					refresh();

			bool					openRead(xfilestream& outFileStream);
			bool					openWrite(xfilestream& outFileStream);

			u64						readAllBytes(xbyte* buffer, u64 count);
			u64						writeAllBytes(const xbyte* buffer, u64 count);

			bool					copy(const xfilepath& toFilename, bool overwrite);
			bool					move(const xfilepath& toFilename);

			const xdevicealias*		getAlias() const;

			void					getPath(xdirinfo& outInfo) const;
			bool					getRoot(xdirinfo& outInfo) const;
			bool					getParent(xdirinfo& outInfo) const;
			void					getSubDir(const char* subDir, xdirinfo& outInfo) const;
	
			xfileinfo&				onlyFilename();

			xfileinfo				getFilename() const;
			void					up();
			void					down(const char* subDir);

			bool					getTime(xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const;
			bool					getCreationTime  (xdatetime&) const;
			bool					getLastAccessTime(xdatetime&) const;
			bool					getLastWriteTime (xdatetime&) const;
			bool					setTime(const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime);
			bool					setCreationTime(const xdatetime&);
			bool					setLastAccessTime(const xdatetime&);
			bool					setLastWriteTime (const xdatetime&);

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

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_FILE_INFO_H__
//==============================================================================
#endif