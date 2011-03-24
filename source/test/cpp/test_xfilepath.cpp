#include "xbase\x_types.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"

using namespace xcore;
using namespace xfilesystem;

UNITTEST_SUITE_BEGIN(filepath)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(constructor1)
		{
			const char* str = "This is a test string";

			xfilepath p(str);

			CHECK_FALSE(p.empty());
			CHECK_TRUE(p.length() == x_strlen(str));
			CHECK_EQUAL(p.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p.c_str(), str), 0);
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "This is a test string";

			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xfilepath p2(p1);

			CHECK_FALSE(p2.empty());
			CHECK_TRUE(p2.length() == x_strlen(str));
			CHECK_EQUAL(p2.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p2.c_str(), str), 0);
		}

		UNITTEST_TEST(constructor3)
		{
			xfilepath p1;

			CHECK_TRUE(p1.empty());
			CHECK_TRUE(p1.length() == 0);
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
		}

		UNITTEST_TEST(constructor4)
		{
			const char* str = "This is a test 1 string";

			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xfilepath p2(p1);

			CHECK_FALSE(p2.empty());
			CHECK_TRUE(p2.length() == x_strlen(str));
			CHECK_EQUAL(p2.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p2.c_str(), str), 0);
		}

		UNITTEST_TEST(clear)
		{
			const char* str = "Test";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());

			p1.clear();
			CHECK_TRUE(p1.empty());
			CHECK_TRUE(p1.length() == 0);
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
		}

		UNITTEST_TEST(relative)
		{
			const char* str = "memory:\\folder\\filename.ext";
			xfilepath p1(str);

			const char* relative_p1 = p1.relative();
			CHECK_EQUAL(0, x_strCompare(relative_p1, "folder\\filename.ext"));
		}

		UNITTEST_TEST(setDeviceName)
		{
			const char* str = "memory:\\folder\\filename.ext";
			xfilepath p1(str);

			p1.setDeviceName("remotesource");
			CHECK_EQUAL(0, x_strCompare(p1.c_str(), "remotesource:\\folder\\filename.ext"));
			p1.setDeviceName("localcache");
			CHECK_EQUAL(0, x_strCompare(p1.c_str(), "localcache:\\folder\\filename.ext"));
			p1.setDeviceName(NULL);
			CHECK_EQUAL(0, x_strCompare(p1.c_str(), "folder\\filename.ext"));
		}

		UNITTEST_TEST(setDevicePart)
		{
			const char* str = "memory:\\folder\\filename.ext";
			xfilepath p1(str);

			p1.setDevicePart("remotesource:\\");
			CHECK_EQUAL(0, x_strCompare(p1.c_str(), "remotesource:\\folder\\filename.ext"));
			p1.setDevicePart("localcache:\\");
			CHECK_EQUAL(0, x_strCompare(p1.c_str(), "localcache:\\folder\\filename.ext"));
		}

		UNITTEST_TEST(assignment_operator)
		{
			const char* str = "Test";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());

			const char* str2 = "Test2";
			p1 = str2;
			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str2));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), str2), 0);
		}

		UNITTEST_TEST(plus_assignment_operator)
		{
			const char* str = "Test1";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			const char* str2 = "Test2";
			const char* str12 = "Test1Test2";
			p1 += str2;
			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str12));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), str12), 0);
		}

		UNITTEST_TEST(equal_operator)
		{
			const char* str = "Test1";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xfilepath p2(p1);
			CHECK_EQUAL(p1.empty(), p2.empty());
			CHECK_EQUAL(p1.length(), p2.length());
			CHECK_EQUAL(p1.maxLength(), p2.maxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), p2.c_str()), 0);
		}

		UNITTEST_TEST(not_equal_operator)
		{
			const char* str1 = "Test1";
			xfilepath p1(str1);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str1));
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), str1), 0);

			const char* str2 = "Test11";
			xfilepath p2(str2);
			CHECK_EQUAL(p1.empty(), p2.empty());
			CHECK_NOT_EQUAL(p1.length(), p2.length());
			CHECK_EQUAL(p1.maxLength(), xfilepath::sMaxLength());
			CHECK_EQUAL(x_strCompare(p2.c_str(), str2), 0);
			CHECK_NOT_EQUAL(x_strCompare(p1.c_str(), p2.c_str()), 0);
		}

		UNITTEST_TEST(index_operator)
		{
			const char* str1 = "Test1";
			xfilepath p1(str1);

			CHECK_EQUAL(p1[0], 'T');
			CHECK_EQUAL(p1[1], 'e');
			CHECK_EQUAL(p1[2], 's');
			CHECK_EQUAL(p1[3], 't');
			CHECK_EQUAL(p1[4], '1');
		}

		UNITTEST_TEST(global_add_operator)
		{
			const char* str1 = "C:\\temp\\folderA\\filename.ext";
			xfilepath f1(str1);
			const char* str2 = "C:\\temp\\folderB\\folderC";
			xdirpath d1(str2);

			xfilepath f2 = d1 + f1;
			CHECK_EQUAL(x_strCompare(f2.c_str(), "C:\\temp\\folderB\\folderC\\temp\\folderA\\filename.ext"), 0);
		}
	}
}
UNITTEST_SUITE_END
