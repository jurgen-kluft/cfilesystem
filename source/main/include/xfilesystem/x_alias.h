#ifndef __X_FILESYSTEM_ALIAS_H__
#define __X_FILESYSTEM_ALIAS_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//==============================================================================
	// xalias
	//==============================================================================

	namespace xfilesystem
	{
		class xfilepath;
		class xfiledevice;

		//------------------------------------------------------------------------------
		// Description:
		//     This class maps an alias to a file device.
		//     This can act as an re-direction and is useful for defining folders
		//     as drives, remap drives etc..
		//
		//     source:\folder\filename.ext, where source = c:\temp
		//     data:\folder\filename.ext, where data = g:\
		//     dvd:\folder\filename.ext, where dvd = g:\
		//     
		//------------------------------------------------------------------------------
		class xalias
		{
		public:
								xalias ();
								xalias (const char* alias, const char* aliasTarget);
								xalias (const char* alias, xfiledevice* device, const char* remap=NULL);

			const char*			alias() const								{ return mAliasStr; }
			const char*			aliasTarget() const							{ return mAliasTargetStr; }
			const char*			remap() const;
			xfiledevice*		device() const;

		private:
			const char*         mAliasStr;									///< data
			const char*         mAliasTargetStr;							///< d
			mutable const char* mRemapStr;									///< e.g. "d:\project\data\bin.pc\", data:\file.txt to d:\project\data\bin.pc\file.txt
			xfiledevice*		mFileDevice;
		};

		extern void				gAddAlias(xalias& inAlias);
		extern const xalias*	gFindAlias(const char* inAlias);
		extern const xalias*	gFindAliasFromFilename(const xfilepath& inFilename);
		extern const xalias*	gFindAndRemoveAliasFromFilename(xfilepath& ioFilename);
		extern void				gReplaceAliasOfFilename(xfilepath& ioFilename, const char* inNewAlias);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_ALIAS_H__
//==============================================================================
#endif