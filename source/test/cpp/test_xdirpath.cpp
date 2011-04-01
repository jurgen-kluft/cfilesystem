#include "xbase\x_types.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\x_dirinfo.h"

using namespace xcore;
using namespace xfilesystem;

UNITTEST_SUITE_BEGIN(dirpath)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(split)
		{
			const char* str = "K:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);

			xdirpath parent;
			xdirpath sub;
			di.split(2, parent, sub);
			CHECK_EQUAL(0, x_strcmp(parent.c_str(), "K:\\the\\name\\"));
			CHECK_EQUAL(0, x_strcmp(sub.c_str(), "is\\johhnywalker\\"));

			di.split(1, parent, sub);
			CHECK_EQUAL(0, x_strcmp(parent.c_str(), "K:\\the\\"));
			CHECK_EQUAL(0, x_strcmp(sub.c_str(), "name\\is\\johhnywalker\\"));
		}
	}
}
UNITTEST_SUITE_END
