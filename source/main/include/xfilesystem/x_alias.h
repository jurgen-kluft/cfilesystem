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
		class xfiledevice;

		//------------------------------------------------------------------------------
		// Description:
		//     This class maps an alias to a file device.
		//     This acts as an indirection and is useful for defining folders
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
			mutable const char* mRemapStr;									///< e.g. "d:\project\data\bin.pc", data:\file.txt to d:\project\data\bin.pc\file.txt
			xfiledevice*		mFileDevice;
		};

		extern void				addAlias(xalias& inAlias);
		extern const xalias*	findAlias(const char* inAlias);
		extern const xalias*	findAliasFromFilename(const char* inFilename);
		extern const xalias*	findAndRemoveAliasFromFilename(char* ioFilename);
		extern void				replaceAliasOfFilename(char* ioFilename, s32 inFilenameMaxLength, const char* inNewAlias);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_ALIAS_H__
//==============================================================================
#endif