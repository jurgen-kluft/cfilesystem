#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"

#include "xunittest/xunittest.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_stream.h"

using namespace xcore;

UNITTEST_SUITE_BEGIN(filepath)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(constructor1)
		{
			xfilepath p1;

			CHECK_TRUE(p1.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "TEST:\\textfiles\\docs";
			xfilepath p = xfilesystem::filepath(str);

			CHECK_FALSE(p.isEmpty());
		}

	}
}
UNITTEST_SUITE_END
