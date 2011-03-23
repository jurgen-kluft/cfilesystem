#include "xbase\x_types.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"

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
			CHECK_EQUAL(p.maxLength(), x_strlen(str));
			CHECK_EQUAL(x_strCompare(p.c_str(), str), 0);
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "This is a test string";

			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xfilepath p2(p1);

			CHECK_FALSE(p2.empty());
			CHECK_TRUE(p2.length() == x_strlen(str));
			CHECK_EQUAL(p2.maxLength(), x_strlen(str));
			CHECK_EQUAL(x_strCompare(p2.c_str(), str), 0);
		}

		UNITTEST_TEST(constructor3)
		{
			xfilepath p1;

			CHECK_TRUE(p1.empty());
			CHECK_TRUE(p1.length() == 0);
			CHECK_EQUAL(p1.maxLength(), 0);
		}

		UNITTEST_TEST(constructor4)
		{
			const char* str = "This is a test 1 string";

			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xfilepath p2(p1);

			CHECK_FALSE(p2.empty());
			CHECK_TRUE(p2.length() == x_strlen(str));
			CHECK_EQUAL(p2.maxLength(), x_strlen(str));
			CHECK_EQUAL(x_strCompare(p2.c_str(), str), 0);
		}

		UNITTEST_TEST(clear)
		{
			const char* str = "Test";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str));

			p1.clear();
			CHECK_TRUE(p1.empty());
			CHECK_TRUE(p1.length() == 0);
			CHECK_EQUAL(p1.maxLength(), x_strlen(str));
		}

		UNITTEST_TEST(find)
		{
			const char* str = "memory:\\folder\\filename.ext";
			xfilepath p1(str);

			s32 pos1 = p1.find(":\\");
			CHECK_EQUAL(6, pos1);
			s32 pos2 = p1.find("AA");
			CHECK_EQUAL(-1, pos2);
		}

		UNITTEST_TEST(replace)
		{
			const char* str = "memory:\\folder\\filename.ext";
			xfilepath p1(str);

			p1.replace(8, 6, "longerfolder");
			CHECK_TRUE(x_strCompare(p1.c_str(), "memory:\\longerfolder\\filename.ext") == 0);
			p1.replace(8, 12, "fldr");
			CHECK_TRUE(x_strCompare(p1.c_str(), "memory:\\fldr\\filename.ext") == 0);
		}

		UNITTEST_TEST(assignment_operator)
		{
			const char* str = "Test";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str));

			const char* str2 = "Test2";
			p1 = str2;
			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str2));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str2));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str2), 0);
		}

		UNITTEST_TEST(plus_assignment_operator)
		{
			const char* str = "Test1";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			const char* str2 = "Test2";
			const char* str12 = "Test1Test2";
			p1 += str2;
			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str12));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str12));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str12), 0);
		}

		UNITTEST_TEST(equal_operator)
		{
			const char* str = "Test1";
			xfilepath p1(str);

			CHECK_FALSE(p1.empty());
			CHECK_TRUE(p1.length() == x_strlen(str));
			CHECK_EQUAL(p1.maxLength(), x_strlen(str));
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
			CHECK_EQUAL(p1.maxLength(), x_strlen(str1));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str1), 0);

			const char* str2 = "Test11";
			xfilepath p2(str2);
			CHECK_EQUAL(p1.empty(), p2.empty());
			CHECK_NOT_EQUAL(p1.length(), p2.length());
			CHECK_NOT_EQUAL(p1.maxLength(), p2.maxLength());
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
	}
}
UNITTEST_SUITE_END
