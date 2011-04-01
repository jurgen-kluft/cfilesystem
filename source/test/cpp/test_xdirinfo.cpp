#include "xbase\x_types.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\x_dirinfo.h"

using namespace xcore;
using namespace xfilesystem;

UNITTEST_SUITE_BEGIN(dirinfo)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(constructor1)
		{
			const char* str = "K:\\xfilesystem_test";
			xdirinfo di(str);
			CHECK_EQUAL(0, x_strcmp(di.getFullName().c_str(), "K:\\xfilesystem_test\\"));

			const char* str2 = "xfilesystem_test";
			xdirinfo di2(str2);
			CHECK_EQUAL(0, x_strcmp(di2.getFullName().c_str(), "xfilesystem_test\\"));
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "K:\\xfilesystem_test\\";
			xdirinfo di(str);
			xdirinfo di2(di);
		
			CHECK_EQUAL(0, x_strcmp(di2.getFullName().c_str(), str));
		}

		UNITTEST_TEST(getName)
		{
			const char* str = "K:\\xfilesystem_test\\the_folder";
			xdirinfo di(str);

			char nameBuffer[xdirpath::XDIR_MAX_PATH];
			xcstring name(nameBuffer, sizeof(nameBuffer));
			di.getName(name);

			CHECK_EQUAL(0, x_strCompareNoCase("the_folder", nameBuffer));
		}

		UNITTEST_TEST(isValid)
		{
			const char* str = "K:\\xfilesystem_test\the_folder";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isValid());

			const char* str2 = "INVALID:\\xfilesystem_test\the_folder";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isValid());
		}

		UNITTEST_TEST(isRoot)
		{
			const char* str = "K:\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isRoot());

			const char* str2 = "";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isRoot());

			const char* str3 = "xfilesystem_test";
			xdirinfo di3(str3);
			CHECK_EQUAL(false, di3.isRoot());

			const char* str4 = "K:\\xfilesystem_test";
			xdirinfo di4(str4);
			CHECK_EQUAL(false, di4.isRoot());
		}

		UNITTEST_TEST(isRooted)
		{
			const char* str = "K:\\xfilesystem_test\the_folder";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isRooted());

			const char* str2 = "xfilesystem_test\the_folder";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isRooted());
		}

		UNITTEST_TEST(exists)
		{
			const char* str = "K:\\xfilesystem_test\the_folder";
			xdirinfo di(str);
			CHECK_EQUAL(false, di.exists());

			const char* str1 = "INVALID:\\xfilesystem_test\the_folder";
			xdirinfo di1(str1);
			CHECK_EQUAL(false, di1.exists());

			const char* str2 = "K:\\xfilesystem_test";
			xdirinfo di2(str2);
			CHECK_EQUAL(true, di2.exists());
		}
	}
}
UNITTEST_SUITE_END
