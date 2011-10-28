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
			xfilepath p1;

			CHECK_TRUE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == 0);
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "TEST:\\textfiles\\docs";
			xfilepath p(str);

			CHECK_FALSE(p.isEmpty());
			CHECK_TRUE(p.getLength() == x_strlen(str));
			CHECK_EQUAL(x_strCompare(p.c_str(), str), 0);


			const char* str2 = "\\test\\docs\\ad";
			xfilepath p2(str2);
			CHECK_EQUAL(x_strCompare(p2.c_str(),"test\\docs\\ad"),0);

			const char* str3 = "test\\doc\\do\\";
			xfilepath p3(str3);
			CHECK_EQUAL(x_strCompare(p3.c_str(),"test\\doc\\do"),0);
		}

		UNITTEST_TEST(constructor3)
		{
			const char* str = "TEST:\\textfiles\\docs";
			xfilepath p1(str);

			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xfilepath p2(p1);

			CHECK_FALSE(p2.isEmpty());
			CHECK_TRUE(p2.getLength() == x_strlen(str));
			CHECK_EQUAL(x_strCompare(p2.c_str(), str), 0);
		}

		UNITTEST_TEST(constructor4)
		{
			const char* str = "TEST:\\docs\\readme.txt";
			xfilepath p1(str);

			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xdirpath d1("TEST:\\textfiles");
			const char* res = "TEST:\\textfiles\\docs\\readme.txt";
			xfilepath p2(d1, p1);

			CHECK_FALSE(p2.isEmpty());
			CHECK_TRUE(p2.getLength() == x_strlen(res));
			CHECK_EQUAL(x_strCompare(p2.c_str(), res), 0);
		}

		UNITTEST_TEST(clear)
		{
			const char* str = "TEST:\\docs";
			xfilepath p1(str);

			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str));

			p1.clear();
			CHECK_TRUE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == 0);
		}
		
		UNITTEST_TEST(getLength)
		{
			const char* str = "TEST:\\docs";
			xfilepath p1(str);

			CHECK_EQUAL(x_strlen(str), p1.getLength());
		}

		UNITTEST_TEST(getMaxLength)
		{
			const char* str = "TEST:\\docs";
			xfilepath p1(str);

			CHECK_EQUAL(xfilepath::XFILEPATH_MAX, p1.getMaxLength());
			CHECK_NOT_EQUAL(x_strlen(str), p1.getMaxLength());
		}

		UNITTEST_TEST(isEmpty)
		{
			const char* str1 = "TEST:\\docs";
			xfilepath p1(str1);
			CHECK_FALSE(p1.isEmpty());

			xfilepath p2;
			CHECK_TRUE(p2.isEmpty());
		}

		UNITTEST_TEST(isRooted)
		{
			const char* str1 = "TEST:\\docs";
			xfilepath p1(str1);
			CHECK_TRUE(p1.isRooted());

			const char* str2 = "docs\\test.txt";
			xfilepath p2(str2);
			CHECK_FALSE(p2.isRooted());
		}

		UNITTEST_TEST(relative)
		{
			const char* str = "TEST:\\folder\\filename.ext";
			xfilepath p1(str);

			xfilepath r1;
			p1.relative(r1);
			CHECK_EQUAL(0, x_strCompare(r1.c_str(), "folder\\filename.ext"));
		}

		UNITTEST_TEST(makeRelative)
		{
			const char* str1 = "Test:\\makeRelative\\test\\test.txt";
			xfilepath p1(str1);
			p1.makeRelative();
			const char* str2 = "makeRelative\\test\\test.txt";
			xfilepath p2(str2);
			CHECK_TRUE(p1 == p2);

			xfilepath p3(p2);
			p3.makeRelative();
			CHECK_TRUE(p2 == p3);
		}

		UNITTEST_TEST(onlyFilename)
		{
			const char* str1 = "Test:\\onlyFilename\\test\\test.txt";
			xfilepath p1(str1);
			const char* str2 = "test.txt";
			xfilepath p2(str2);
			p1.onlyFilename();
			CHECK_TRUE(p1 == p2);
		}

		UNITTEST_TEST(getFilename)
		{
			const char* str1 = "Test:\\onlyFilename\\test\\test.txt";
			xfilepath p1(str1);
			const char* str2 = "test.txt";
			xfilepath p2(str2);
			CHECK_TRUE(p1.getFilename() == p2);
		}

		UNITTEST_TEST(up)
		{
			const char* str1 = "Test:\\onlyFilename\\test\\test.txt";
			xfilepath p1(str1);
			const char* str2 = "Test:\\onlyFilename\\test.txt";
			xfilepath p2(str2);
			p1.up();
			CHECK_TRUE(p1 == p2);
		}

		UNITTEST_TEST(down)
		{
			const char* str1 = "Test:\\onlyFilename\\test\\test.txt";
			xfilepath p1(str1);
			const char* str2 = "Test:\\onlyFilename\\test\\down\\test.txt";
			xfilepath p2(str2);
			p1.down("down");
			CHECK_TRUE(p1 == p2);

			const char* str3 = "only.txt";
			xfilepath p3(str3);
			const char* str4 = "down\\only.txt";
			xfilepath p4(str4);
			p3.down("down");
			CHECK_TRUE(p3 == p4);
		}

		UNITTEST_TEST(getName)
		{
			const char* str1 = "Test:\\onlyFilename\\test.txt";
			xfilepath p1(str1);
			char buffer1[10];
			char buffer2[10];
			xcstring string1(buffer1,sizeof(buffer1));
			xcstring string2(buffer2,sizeof(buffer2),"test");
			p1.getName(string1);
			CHECK_EQUAL(0,x_strCompare(string1.c_str(),string2.c_str()));
		}

		UNITTEST_TEST(getExtension)
		{
			const char* str1 = "Test:\\onlyFilename\\test.txt";
			xfilepath p1(str1);
			char buffer1[10];
			char buffer2[10];
			xcstring string1(buffer1,sizeof(buffer1));
			xcstring string2(buffer2,sizeof(buffer2),".txt");
			p1.getExtension(string1);
			CHECK_EQUAL(0,x_strCompare(string1.c_str(),string2.c_str()));
		}

		UNITTEST_TEST(getSystem)
		{
			const char* str1 = "TEST:\\getSystem\\test.txt";
			const char* str2 = "TEST:/getSystem/test.txt";
			xfilepath p1(str1);
			char buffer1[256];
			xcstring string1(buffer1,sizeof(buffer1));
			CHECK_NOT_NULL(p1.getSystem(string1));
#if defined(TARGET_PC) || defined(TARGET_360)
			CHECK_EQUAL(0,x_strCompare(string1.c_str(),str1));
#endif
#if defined(TARGET_PS3) || defined(TARGET_PSP) || defined(TARGET_WII)
			CHECK_EQUAL(0,x_strCompare(string1.c_str(),str2));
#endif

			x_printf("getSystem() in test xfilepath returned: %s\n", string1.c_str());
			
		}

		UNITTEST_TEST(getDirPath)
		{
			const char* str1 = "Test:\\getDirPath\\test.txt";
			xfilepath p1(str1);
			const char* str2 = "Test:\\getDirPath";
			xdirpath dp2(str2);
			xdirpath dp1;
			p1.getDirPath(dp1);
			CHECK_TRUE(dp1 == dp2);
		}

		UNITTEST_TEST(getRoot)
		{
			const char* str1 = "Test:\\getRoot\\test.txt";
			xfilepath p1(str1);
			const char* str2 = "Test:\\";
			xdirpath dp2(str2);
			xdirpath dp1;
			p1.getRoot(dp1);
			CHECK_TRUE(dp1 == dp2);
		}

		UNITTEST_TEST(getParent)
		{
			const char* str1 = "Test:\\te\\getParent\\test.txt";
			xfilepath p1(str1);
			const char* str2 = "Test:\\te";
			xdirpath dp2(str2);
			xdirpath dp1;
			p1.getParent(dp1);
			CHECK_TRUE(dp1 == dp2);
		}

		UNITTEST_TEST(getSubDir)
		{
			const char* str1 = "Test:\\getSubDir\\test.txt";
			xfilepath p1(str1);
			xdirpath dp1;
			const char* str2 = "Test:\\getSubDir\\Sub";
			xdirpath dp2(str2);
			p1.getSubDir("Sub",dp1);
			CHECK_TRUE(dp1 == dp2);
		}

		UNITTEST_TEST(c_str)
		{
			const char* str1 = "Test:\\getSubDir\\test.txt";
			xfilepath p1(str1);
			CHECK_EQUAL(0,x_strCompare(p1.c_str(),str1));
		}

		UNITTEST_TEST(setDeviceName)
		{
			const char* str = "TEST:\\folder\\filename.ext";
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
			const char* str = "TEST:\\folder\\filename.ext";
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

			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str));

			const char* str2 = "Test2";
			p1 = str2;
			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str2));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str2), 0);
		}

		UNITTEST_TEST(plus_assignment_operator)
		{
			const char* str = "Test1";
			xfilepath p1(str);

			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			const char* str2 = "Test2";
			const char* str12 = "Test1Test2";
			p1 += str2;
			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str12));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str12), 0);
		}

		UNITTEST_TEST(equal_operator)
		{
			const char* str = "Test1";
			xfilepath p1(str);

			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str), 0);

			xfilepath p2(p1);
			CHECK_EQUAL(p1.isEmpty(), p2.isEmpty());
			CHECK_EQUAL(p1.getLength(), p2.getLength());
			CHECK_EQUAL(p1.getMaxLength(), p2.getMaxLength());
			CHECK_EQUAL(x_strCompare(p1.c_str(), p2.c_str()), 0);
		}

		UNITTEST_TEST(not_equal_operator)
		{
			const char* str1 = "Test1";
			xfilepath p1(str1);

			CHECK_FALSE(p1.isEmpty());
			CHECK_TRUE(p1.getLength() == x_strlen(str1));
			CHECK_EQUAL(x_strCompare(p1.c_str(), str1), 0);

			const char* str2 = "Test11";
			xfilepath p2(str2);
			CHECK_EQUAL(p1.isEmpty(), p2.isEmpty());
			CHECK_NOT_EQUAL(p1.getLength(), p2.getLength());
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
			const char* str1 = "TEST:\\temp\\folderA\\filename.ext";
			xfilepath f1(str1);
			const char* str2 = "TEST:\\temp\\folderB\\folderC";
			xdirpath d1(str2);

			xfilepath f2 = d1 + f1;
			CHECK_EQUAL(x_strCompare(f2.c_str(), "TEST:\\temp\\folderB\\folderC\\temp\\folderA\\filename.ext"), 0);
		}
	}
}
UNITTEST_SUITE_END
