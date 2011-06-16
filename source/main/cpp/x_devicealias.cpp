//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\private\x_devicealias.h"
#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	//----------------------- xdevicealias Implementation ------------------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------

	namespace xfilesystem
	{
		enum EFileSystemConfig
		{
			MAX_FILE_ALIASES = 64,
		};


		static s32				sNumAliases = 0;
		static xdevicealias		sAliasList[MAX_FILE_ALIASES];

		//------------------------------------------------------------------------------
		void				xdevicealias::init()
		{
			sNumAliases = 0;
			for (s32 i=0; i<MAX_FILE_ALIASES; ++i)
				sAliasList[i] = xdevicealias();
		}

		//------------------------------------------------------------------------------

		void				xdevicealias::exit()
		{
			sNumAliases = 0;
		}

		//==============================================================================
		// Functions
		//==============================================================================
		xdevicealias::xdevicealias()
			: mAliasStr(NULL)
			, mAliasTargetStr(NULL)
			, mRemapStr(NULL)
			, mFileDevice(NULL)
		{
		}

		//------------------------------------------------------------------------------

		xdevicealias::xdevicealias(const char* alias, const char* aliasTarget)
			: mAliasStr(alias)
			, mAliasTargetStr(aliasTarget)
			, mRemapStr(NULL)
			, mFileDevice(NULL)
		{
		}

		//------------------------------------------------------------------------------

		xdevicealias::xdevicealias(const char* alias, xfiledevice* device, const char* remap)
			: mAliasStr(alias)
			, mAliasTargetStr(NULL)
			, mRemapStr(remap)
			, mFileDevice(device)
		{
		}

		//------------------------------------------------------------------------------

		const char* xdevicealias::remap() const
		{
			if (mAliasTargetStr != NULL)
			{
				const xdevicealias* a = sFind(mAliasTargetStr);
				if (a == NULL)
					return mRemapStr;
				mRemapStr = a->remap();
			}
			return mRemapStr;
		}

		//------------------------------------------------------------------------------

		xfiledevice* xdevicealias::device() const
		{
			if (mAliasTargetStr != NULL)
			{
				const xdevicealias* a = sFind(mAliasTargetStr);
				if (a == NULL)
					return NULL;
				return a->device();
			}
			return mFileDevice;
		}

		//------------------------------------------------------------------------------

		bool				xdevicealias::sRegister(const xdevicealias& alias)
		{
			for (s32 i=0; i<sNumAliases; ++i)
			{
				if (x_stricmp(sAliasList[i].alias(), alias.alias()) == 0)
				{
					sAliasList[i] = alias;
					xconsole::writeLine("INFO replaced xdevicealias %s", x_va_list(alias.alias()));
					return true;
				}
			}

			if (sNumAliases < MAX_FILE_ALIASES)
			{
				sAliasList[sNumAliases] = alias;
				sNumAliases++;
				return true;
			}
			else
			{
				xconsole::writeLine("ERROR cannot add another xdevicealias, maximum amount of aliases reached");
				return false;
			}
		}

		//------------------------------------------------------------------------------

		const xdevicealias*		xdevicealias::sFind(const char* _alias)
		{
			for (s32 i=0; i<sNumAliases; ++i)
			{
				if (x_strCompareNoCase(sAliasList[i].alias(), _alias) == 0)
				{
					return &sAliasList[i];
				}
			}
			return NULL;
		}

		//------------------------------------------------------------------------------

		const xdevicealias*		xdevicealias::sFind(const xfilepath& inFilename)
		{
			char deviceStrBuffer[64];
			xcstring deviceName(deviceStrBuffer, sizeof(deviceStrBuffer));
			inFilename.getDeviceName(deviceName);
			const xdevicealias* alias = sFind(deviceStrBuffer);
			return alias;
		}

		//------------------------------------------------------------------------------

		const xdevicealias*		xdevicealias::sFind(const xdirpath& inDirectory)
		{
			char deviceStrBuffer[64];
			xcstring deviceName(deviceStrBuffer, sizeof(deviceStrBuffer));
			inDirectory.getDeviceName(deviceName);
			const xdevicealias* alias = sFind(deviceStrBuffer);
			return alias;
		}


		bool		x_RegisterAlias (const char* alias, const char* aliasTarget)
		{
			xdevicealias device_alias(alias, aliasTarget);
			return xdevicealias::sRegister(device_alias);
		}

		bool		x_RegisterAlias (const char* alias, xfiledevice* device, const char* remap)
		{
			xdevicealias device_alias(alias, device, remap);
			return xdevicealias::sRegister(device_alias);
		}


	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
