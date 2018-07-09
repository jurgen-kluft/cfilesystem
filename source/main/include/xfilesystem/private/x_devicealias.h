#ifndef __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#define __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xfilesystem\x_devicealias.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//==============================================================================
	// xdevicealias
	//==============================================================================

	namespace xfilesystem
	{
		class xdirpath;
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
		class xdevicealias
		{
		public:
										xdevicealias ();
										xdevicealias (const char* alias, const char* aliasTarget);
										xdevicealias (const char* alias, xfiledevice* device, const char* remap=NULL);

			const char*					alias() const								{ return mAliasStr; }
			const char*					aliasTarget() const							{ return mAliasTargetStr; }
			const char*					remap() const;
			xfiledevice*				device() const;

			static bool					sRegister(const xdevicealias& inAlias);

			static const xdevicealias*	sFind(const char* inAlias);
			static const xdevicealias*	sFind(const xfilepath& inAlias);
			static const xdevicealias*	sFind(const xdirpath& inAlias);

			static void					init();
			static void					exit();

		private:
			const char*					mAliasStr;									///< data
			const char*					mAliasTargetStr;							///< d
			mutable const char*			mRemapStr;									///< e.g. "d:\project\data\bin.pc\", data:\file.txt to d:\project\data\bin.pc\file.txt
			xfiledevice*				mFileDevice;
		};

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_DEVICE_ALIAS_PRIVATE_H__
//==============================================================================
#endif