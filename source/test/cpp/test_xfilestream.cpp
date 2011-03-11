#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_filestream.h"

using namespace xcore;
using namespace xfilesystem;

UNITTEST_SUITE_BEGIN(filestream)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(open)
		{
			char filenameBuffer[256];
			xfilestream fs(xfilepath(filenameBuffer, sizeof(filenameBuffer), "memory:\\file.txt"), FileMode_Open, FileAccess_Read);
		}

	}
}
UNITTEST_SUITE_END
