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

UNITTEST_SUITE_BEGIN(dirpath)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		static const char*		sFolders[] = {
			"the",
			"name",
			"is",
			"johhnywalker",
		};

		UNITTEST_TEST(constructor1)
		{
			dirpath_t dirpath;
			CHECK_EQUAL(true,dirpath.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
			dirpath_t dirpath = filesystem_t::dirpath("C:\\the\\name\\is\\johhnywalker");
			CHECK_EQUAL(false, dirpath.isEmpty());
		}
	}
}
UNITTEST_SUITE_END
