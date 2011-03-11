//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_alias.h"
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
			MAX_FILE_ALIASES = 32,
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
				const xalias* a = findAlias(mAliasTargetStr);
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
				const xalias* a = findAlias(mAliasTargetStr);
				if (a == NULL)
					return NULL;
				return a->device();
			}

			return mFileDevice;
		}

		//------------------------------------------------------------------------------

		void            addAlias(xalias& alias)
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

		const xalias* findAlias(const char* _alias)
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

		const xalias* findAliasFromFilename(const char* inFilename)
		{
			char deviceStrBuffer[32];

			const char* pos = x_strFind(inFilename, ":\\");
			if (pos!=NULL)
			{
				char* dst = deviceStrBuffer;
				const char* src = inFilename;
				while (src < pos)
					*dst++ = *src++;
				*dst = '\0';
			}

			const xalias* alias = findAlias(deviceStrBuffer);
			return alias;
		}

		//------------------------------------------------------------------------------

		const xalias*		findAndRemoveAliasFromFilename(char* ioFilename)
		{
			const xalias* alias = findAliasFromFilename(ioFilename);
			replaceAliasOfFilename(ioFilename, 0 /*x_strlen(ioFilename)*/, NULL);
			return alias;
		}

		//------------------------------------------------------------------------------

		void				replaceAliasOfFilename(char* ioFilename, s32 inFilenameMaxLength, const char* inNewAlias)
		{
			const char* pos1 = x_strFind(ioFilename, ":\\");
			if (pos1!=NULL)
			{
				const s32 aliaslen1 = (s32)(pos1 - ioFilename);
				const s32 aliaslen2 = inNewAlias!=NULL ? (s32)x_strlen(inNewAlias) : 0;

				if (aliaslen1 >= aliaslen2 || inNewAlias==NULL)
				{
					const char* src = ioFilename + aliaslen1;
					if (inNewAlias==NULL)
						src += 2;	// Remove ':\'

					char* dst = ioFilename + aliaslen2;
					while (*src != '\0')
						*dst++ = *src++;
				}
				else
				{
					const s32 len = x_strlen(ioFilename);

					char* end1 = ioFilename + len;
					char* end2 = ioFilename + len + (aliaslen2 - aliaslen1);

					// Guard against maximum string length of ioFilename
					if ((len + (aliaslen2 - aliaslen1)) > inFilenameMaxLength)
						return;

					while (end1 >= pos1)
						*end2-- = *end1--;
				}
			}
			else if (inNewAlias!=NULL)
			{
				// Make space for alias
				const s32 len1 = x_strlen(ioFilename);
				const s32 len2 = x_strlen(inNewAlias) + 2;

				const char* end1 = ioFilename + len1;
				char* end2 = ioFilename + len1 + len2;

				// Guard against maximum string length of ioFilename
				if ((len1 + len2) > inFilenameMaxLength)
					return;

				while (end1 >= pos1)
					*end2-- = *end1--;

				*end2-- = '\\';
				*end2-- = ':';
			}

			// Copy in new alias
			if (inNewAlias!=NULL)
			{
				const char* src = inNewAlias;
				char* dst = ioFilename;
				while (*src != '\0')
					*dst++ = *src++;
			}

		}
	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
