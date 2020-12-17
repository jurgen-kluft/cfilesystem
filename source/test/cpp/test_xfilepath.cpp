#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"

#include "xunittest/xunittest.h"

#include "filesystem_t/private/x_filedevice.h"
#include "filesystem_t/x_filesystem.h"
#include "filesystem_t/x_filepath.h"
#include "filesystem_t/x_dirpath.h"
#include "filesystem_t/x_dirinfo.h"
#include "filesystem_t/x_fileinfo.h"
#include "filesystem_t/x_stream.h"

using namespace xcore;

UNITTEST_SUITE_BEGIN(filepath)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(constructor1)
		{
			filepath_t p1;

			CHECK_TRUE(p1.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "TEST:\\textfiles\\docs";
			filepath_t p = filesystem_t::filepath(str);

			CHECK_FALSE(p.isEmpty());
		}

	}
}
UNITTEST_SUITE_END
