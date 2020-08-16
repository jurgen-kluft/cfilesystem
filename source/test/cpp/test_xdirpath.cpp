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
			xdirpath dirpath;
			CHECK_EQUAL(true,dirpath.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
			xdirpath dirpath = xfilesystem::dirpath("C:\\the\\name\\is\\johhnywalker");
			CHECK_EQUAL(false, dirpath.isEmpty());
		}
	}
}
UNITTEST_SUITE_END
