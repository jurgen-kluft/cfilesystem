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
	namespace xfilesystem
	{
		class xpath;
		class xdirpath;
		class xfilesystem;
		class xfiledevice;

		//==============================================================================
		// xfilepath: 
		//		- Relative:		Folder\Filename.Extension
		//		- Absolute:		Device:\Folder\Folder\Filename.Extension
		//==============================================================================
		class xfilepath
		{
		protected:
			friend class xdirpath;

			xfilesystem*			mParent;
			xstring					mString;

		public:
									xfilepath();
									xfilepath(xstring const& str);
									xfilepath(const xfilepath& filepath);
			explicit				xfilepath(const xdirpath& dir, const xfilepath& filename);
									~xfilepath();

			void					clear();

			bool					isEmpty() const;
			bool					isRooted() const;

			void					relative(xfilepath&) const;
			void					makeRelative();
			
			xstring					getFilepath() const;
			xstring					getFilename() const;
			xstring					getFilenameWithoutExtension() const;
			xstring					getExtension() const;
			xdirpath				getDirname() const;

			xfiledevice*			getSystem(xcstring& outSystemFilePath) const;

			xfilepath&				operator =  (const xstring&);
			xfilepath&				operator =  (const xfilepath&);
			xfilepath&				operator += (const xstring&);
			xfilepath&				operator += (const xfilepath&);

			bool					operator == (const xfilepath&) const;
			bool					operator != (const xfilepath&) const;

		private:
			void					fixSlashes();														///< Fix slashes, replace '/' with '\'. For Unix, replace '\' with '/'.		
			void					fixSlashesForDevice();
		};

		inline xfilepath	operator + (const xdirpath& dir, const xfilepath& filename)	{ return xfilepath(dir, filename); }

	};

};

#endif