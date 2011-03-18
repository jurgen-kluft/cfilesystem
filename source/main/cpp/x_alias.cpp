//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_alias.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	//----------------------- xalias Implementation ------------------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------

	namespace xfilesystem
	{
		enum EFileSystemConfig
		{
			MAX_FILE_ALIASES = 64,
		};


		static s32			sNumAliases = 0;
		static xalias		sAliasList[MAX_FILE_ALIASES];

		//------------------------------------------------------------------------------
		void				initAlias()
		{
			sNumAliases = 0;
			for (s32 i=0; i<MAX_FILE_ALIASES; ++i)
				sAliasList[i] = xalias();
		}

		//------------------------------------------------------------------------------

		void				exitAlias()
		{
			sNumAliases = 0;
		}

		//==============================================================================
		// Functions
		//==============================================================================
		xalias::xalias()
			: mAliasStr(NULL)
			, mAliasTargetStr(NULL)
			, mRemapStr(NULL)
			, mFileDevice(NULL)
		{
		}

		//------------------------------------------------------------------------------

		xalias::xalias(const char* alias, const char* aliasTarget)
			: mAliasStr(alias)
			, mAliasTargetStr(aliasTarget)
			, mRemapStr(NULL)
			, mFileDevice(NULL)
		{
		}

		//------------------------------------------------------------------------------

		xalias::xalias(const char* alias, xfiledevice* device, const char* remap)
			: mAliasStr(alias)
			, mAliasTargetStr(NULL)
			, mRemapStr(remap)
			, mFileDevice(device)
		{
		}

		//------------------------------------------------------------------------------

		const char* xalias::remap() const
		{
			if (mAliasTargetStr != NULL)
			{
				const xalias* a = gFindAlias(mAliasTargetStr);
				if (a == NULL)
					return mRemapStr;
				mRemapStr = a->remap();
			}
			return mRemapStr;
		}

		//------------------------------------------------------------------------------

		xfiledevice* xalias::device() const
		{
			if (mAliasTargetStr != NULL)
			{
				const xalias* a = gFindAlias(mAliasTargetStr);
				if (a == NULL)
					return NULL;
				return a->device();
			}
			return mFileDevice;
		}

		//------------------------------------------------------------------------------

		void				gAddAlias(xalias& alias)
		{
			for (s32 i=0; i<sNumAliases; ++i)
			{
				if (x_stricmp(sAliasList[i].alias(), alias.alias()) == 0)
				{
					sAliasList[i] = alias;
					xconsole::writeLine("INFO replaced alias %s", x_va_list(alias.alias()));
					return;
				}
			}

			if (sNumAliases < MAX_FILE_ALIASES)
			{
				sAliasList[sNumAliases] = alias;
				sNumAliases++;
			}
			else
			{
				xconsole::writeLine("ERROR cannot add another xfilesystem alias, maximum amount of aliases reached");
			}
		}

		//------------------------------------------------------------------------------

		const xalias*		gFindAlias(const char* _alias)
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

		const xalias*		gFindAliasFromFilename(const xfilepath& inFilename)
		{
			char deviceStrBuffer[32];

			const s32 pos = inFilename.find(":\\");
			if (pos >= 0)
			{
				inFilename.subString(0, pos, deviceStrBuffer, sizeof(deviceStrBuffer)-1);
				const xalias* alias = gFindAlias(deviceStrBuffer);
				return alias;
			}

			return NULL;
		}

		//------------------------------------------------------------------------------

		const xalias*		gFindAndRemoveAliasFromFilename(xfilepath& ioFilename)
		{
			const xalias* alias = gFindAliasFromFilename(ioFilename);
			gReplaceAliasOfFilename(ioFilename, alias!=NULL ? alias->alias() : NULL);
			return alias;
		}

		//------------------------------------------------------------------------------

		void				gReplaceAliasOfFilename(xfilepath& ioFilename, const char* inNewAlias)
		{
			const s32 pos = ioFilename.find(":\\");
			if (pos >= 0)
			{
				ioFilename.replace(0, pos, inNewAlias);
			}
			else if (inNewAlias!=NULL)
			{
				ioFilename.replace(0, 0, inNewAlias);
			}
		}
	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
