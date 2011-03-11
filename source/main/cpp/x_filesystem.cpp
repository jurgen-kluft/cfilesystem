//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_fileasync.h"
#include "xfilesystem\private\x_fileinfo.h"
#include "xfilesystem\private\x_filedevice.h"

#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		void IoThreadWorker()
		{
			do
			{
				xfileasync* pAsync = asyncIORemoveHead();
				if (pAsync)
				{
					if (pAsync->getFileIndex() >=  0)
					{
						xfileinfo* pInfo = getFileInfo(pAsync->getFileIndex());
						xfiledevice* device = pInfo->m_pFileDevice;
						device->HandleAsync(pAsync, pInfo);
						freeAsyncIOAddToTail(pAsync);
					}
				}
				else
				{
					ioThreadWait();
				}
			} while (ioThreadLoop());
		}
		
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
