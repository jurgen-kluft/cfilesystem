#ifndef __X_FILESYSTEM_FILE_INFO_H__
#define __X_FILESYSTEM_FILE_INFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
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
		class xfileinfo_imp;

		class xfileinfo
		{
			xfileinfo_imp*			mImplementation;

		public:
									xfileinfo();
									xfileinfo(xfileinfo& fileinfo);
									xfileinfo(const char* filename);
									xfileinfo(xfilepath& filename);

			const xfilepath&		getFullName() const;
			xbool					getName(char* outStr, s32 maxStrLength) const;
			xbool					getExtension(char* outStr, s32 maxStrLength) const;
			u64						getLength() const;

			xbool					isValid() const;
			xbool					isAbsolute() const;
			xbool					isReadOnly() const;

			xbool					exists() const;
			void					create();
			void					create(xfilestream& outFileStream);
			void					remove();
			void					refresh();

			void					openRead(xfilestream& outFileStream);
			void					openWrite(xfilestream& outFileStream);

			void					readAllBytes(xbyte* buffer, s32 offset, s32 count);
			void					writeAllBytes(const xbyte* buffer, s32 offset, s32 count);

			void					copy(xfilepath& destFilename, xbool overwrite=xFALSE);
			void					move(xfilepath& destFilename);

			const xdevicealias*			getAlias() const;
			void					getRoot(xdirinfo& outInfo) const;
			void					getParent(xdirinfo& outInfo) const;

			void					getCreationTime  (xdatetime&) const;
			void					getLastAccessTime(xdatetime&) const;
			void					getLastWriteTime (xdatetime&) const;
			void					setCreationTime  (const xdatetime&) const;
			void					setLastAccessTime(const xdatetime&) const;
			void					setLastWriteTime (const xdatetime&) const;

			void					operator = (const xfileinfo&) const;

			bool					operator == (const xfileinfo&) const;
			bool					operator != (const xfileinfo&) const;

			///< Static functions
			static bool				sExists(xfilepath& filename);
			static u64				sLength(xfilepath& filename);
			static void				sCreate(xfilepath& filename, xfilestream& outFileStream);
			static void				sDelete(xfilepath& filename);

			static void				sOpen(xfilepath& filename, xfilestream& outFileStream);
			static void				sOpenRead(xfilepath& filename, xfilestream& outFileStream);
			static void				sOpenWrite(xfilepath& filename, xfilestream& outFileStream);
			
			///< Opens a binary file, reads the contents of the file into a byte buffer, and then closes the file.
			static void				sReadAllBytes(xfilepath& filename, xbyte* buffer, s32 offset, s32 count);
			///< Creates a new file, writes the specified byte buffer to the file, and then closes the file. If the target file already exists, it is overwritten.
			static void				sWriteAllBytes(xfilepath& filename, const xbyte* buffer, s32 offset, s32 count);

			static void				sSetCreationTime(xfilepath& filename, const xdatetime& creationTime);
			static void				sGetCreationTime(xfilepath& filename, xdatetime& outCreationTime);
			static void				sSetLastAccessTime(xfilepath& filename, const xdatetime& lastAccessTime);
			static void				sGetLastAccessTime(xfilepath& filename, xdatetime& outLastAccessTime);
			static void				sSetLastWriteTime(xfilepath& filename, const xdatetime& lastWriteTime);
			static void				sGetLastWriteTime(xfilepath& filename, xdatetime& outLastWriteTime);

			///< Copies an existing file to a new file. Overwriting a file of the same name is allowed.
			static void				sCopy(xfilepath& sourceFilename, xfilepath& destFilename, xbool overwrite=xFALSE);
			///< Moves a specified file to a new location, providing the option to specify a new file name.
			static void				sMove(xfilepath& sourceFilename, xfilepath& destFilename);
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