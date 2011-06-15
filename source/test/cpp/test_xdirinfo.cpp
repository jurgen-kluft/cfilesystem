#include "xbase\x_types.h"
#include "xbase\x_string_std.h"
#include "xtime\x_datetime.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\x_dirinfo.h"
#include "xfilesystem\x_devicealias.h"

using namespace xcore;
using namespace xfilesystem;

UNITTEST_SUITE_BEGIN(dirinfo)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		static xdatetime sCreationTime(2011, 2, 10, 15, 30, 10);
		static xdatetime sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static xdatetime sLastWriteTime(2011, 2, 11, 10, 46, 20);

		UNITTEST_TEST(constructor1)
		{
			const char* str = "TEST:\\textfiles\\docs";
			xdirinfo di(str);
			CHECK_EQUAL(0, x_strcmp(di.getFullName().c_str(), "TEST:\\textfiles\\docs\\"));

			const char* str2 = "textfiles\\docs";
			xdirinfo di2(str2);
			CHECK_EQUAL(0, x_strcmp(di2.getFullName().c_str(), "textfiles\\docs\\"));
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);
			xdirinfo di2(di);
		
			CHECK_EQUAL(0, x_strcmp(di2.getFullName().c_str(), str));
		}

		UNITTEST_TEST(getName)
		{
			const char* str = "TEST:\\textfiles\\docs\\the_folder";
			xdirinfo di(str);

			char nameBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring name(nameBuffer, sizeof(nameBuffer));
			di.getName(name);

			CHECK_EQUAL(0, x_strCompareNoCase("the_folder", nameBuffer));
		}

		UNITTEST_TEST(isValid)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isValid());

			const char* str2 = "INVALID:\\xfilesystem_test\\the_folder\\";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isValid());
		}

		UNITTEST_TEST(isRoot)
		{
			const char* str = "TEST:\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isRoot());

			const char* str2 = "";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isRoot());

			const char* str3 = "textfiles\\docs";
			xdirinfo di3(str3);
			CHECK_EQUAL(false, di3.isRoot());

			const char* str4 = "TEST:\\textfiles\\docs";
			xdirinfo di4(str4);
			CHECK_EQUAL(false, di4.isRoot());
		}

		UNITTEST_TEST(isRooted)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isRooted());

			const char* str2 = "textfiles\\docs\\";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isRooted());
		}

		UNITTEST_TEST(exists)
		{
			const char* str = "TEST:\\textfiles\\docs\\new_folder\\";
			xdirinfo di(str);
			CHECK_EQUAL(false, di.exists());

			const char* str1 = "INVALID:\\xfilesystem_test\the_folder\\";
			xdirinfo di1(str1);
			CHECK_EQUAL(false, di1.exists());

			const char* str2 = "TEST:\\textfiles\\docs\\";
			xdirinfo di2(str2);
			CHECK_EQUAL(true, di2.exists());
		}

		UNITTEST_TEST(create)
		{
			const char* str = "TEST:\\textfiles\\docs\\new_folder\\";
			xdirinfo di(str);
			CHECK_EQUAL(false, di.exists());
			CHECK_EQUAL(true, di.create());
			CHECK_EQUAL(true, di.exists());
		}

		UNITTEST_TEST(remove)
		{
			const char* str = "TEST:\\textfiles\\docs\\new_folder\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.exists());
			CHECK_EQUAL(true, di.remove());
			CHECK_EQUAL(false, di.exists());
		}

		//		const xdevicealias*		getAlias() const;
		//		bool					getRoot(xdirinfo& outRootDirInfo) const;
		//		bool					getParent(xdirinfo& outParentDirInfo) const;
		//		bool					getSubdir(const char* subDir, xdirinfo& outSubDirInfo) const;
		UNITTEST_TEST(getAlias)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			const xdevicealias* a = di.getAlias();
			CHECK_EQUAL(true, a!=NULL);
			CHECK_EQUAL(0, x_strCompare(a->alias(), "TEST"));
		}

		UNITTEST_TEST(getRoot)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\";
			xdirinfo di1(str1);

			xdirinfo root;
			CHECK_EQUAL(true, di1.getRoot(root));
			CHECK_EQUAL(true, root == "TEST:\\");

			const char* str2 = "textfiles\\docs\\";
			xdirinfo di2(str2);

			CHECK_EQUAL(false, di2.getRoot(root));
		}
		
		UNITTEST_TEST(getParent)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\";
			xdirinfo di1(str1);

			xdirinfo parent;
			CHECK_EQUAL(true, di1.getParent(parent));
			CHECK_EQUAL(true, parent == "TEST:\\textfiles\\");
		}

		UNITTEST_TEST(getSubdir)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirinfo di1(str1);

			xdirinfo sub;
			CHECK_EQUAL(true, di1.getSubdir("docs", sub));
			CHECK_EQUAL(true, sub == "TEST:\\textfiles\\docs\\");
		}

		UNITTEST_TEST(getCreationTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime time;
			CHECK_EQUAL(true, di.getCreationTime(time));
			CHECK_EQUAL(xTRUE, sCreationTime == time);
		}

		UNITTEST_TEST(getLastAccessTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime time;
			CHECK_EQUAL(true, di.getLastAccessTime(time));
			CHECK_EQUAL(xTRUE, sLastAccessTime == time);
		}

		UNITTEST_TEST(getLastWriteTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime time;
			CHECK_EQUAL(true, di.getLastWriteTime(time));
			CHECK_EQUAL(xTRUE, sLastWriteTime == time);
		}

		UNITTEST_TEST(setCreationTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime old_time, set_time(2012, 12, 12, 12, 12, 12), get_time;
			CHECK_EQUAL(true, di.getCreationTime(old_time));
			CHECK_EQUAL(true, di.setCreationTime(set_time));
			CHECK_EQUAL(true, di.getCreationTime(get_time));
			CHECK_EQUAL(xTRUE, set_time == get_time);
			CHECK_EQUAL(true, di.setCreationTime(old_time));
		}

		UNITTEST_TEST(setLastAccessTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime old_time, set_time(2012, 12, 12, 12, 12, 12), get_time;
			CHECK_EQUAL(true, di.getLastAccessTime(old_time));
			CHECK_EQUAL(true, di.setLastAccessTime(set_time));
			CHECK_EQUAL(true, di.getLastAccessTime(get_time));
			CHECK_EQUAL(xTRUE, set_time == get_time);
			CHECK_EQUAL(true, di.setLastAccessTime(old_time));
		}

		UNITTEST_TEST(setLastWriteTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime old_time, set_time(2012, 12, 12, 12, 12, 12), get_time;
			CHECK_EQUAL(true, di.getLastWriteTime(old_time));
			CHECK_EQUAL(true, di.setLastWriteTime(set_time));
			CHECK_EQUAL(true, di.getLastWriteTime(get_time));
			CHECK_EQUAL(xTRUE, set_time == get_time);
			CHECK_EQUAL(true, di.setLastWriteTime(old_time));
		}

		UNITTEST_TEST(assignment_operator1)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);
			const char* str2 = "TEST:\\textfiles2";

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles1\\");

			di1 = str2;

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(assignment_operator2)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);
			const char* str2 = "TEST:\\textfiles2";
			xdirinfo di2(str2);

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles1\\");
			CHECK_EQUAL(true, di2 == "TEST:\\textfiles2\\");

			di1 = di2;

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles2\\");
			CHECK_EQUAL(true, di2 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(assignment_operator3)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);
			const char* str2 = "TEST:\\textfiles2";
			xdirpath dp1(str2);

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles1\\");
			CHECK_EQUAL(true, dp1 == "TEST:\\textfiles2\\");

			di1 = dp1;

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles2\\");
			CHECK_EQUAL(true, dp1 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(equality_operator1)
		{
			const char* str1 = "TEST:\\textfiles1\\";
			xdirinfo di1(str1);

			CHECK_EQUAL(true, di1 == str1);
			CHECK_EQUAL(false, di1 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(equality_operator2)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(true, di1 == xdirpath(str1));
			CHECK_EQUAL(false, di1 == xdirpath("TEST:\\textfiles2\\"));
		}

		UNITTEST_TEST(equality_operator3)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(true, di1 == xdirinfo(str1));
			CHECK_EQUAL(false, di1 == xdirinfo("TEST:\\textfiles2\\"));
		}

		UNITTEST_TEST(non_equality_operator1)
		{
			const char* str1 = "TEST:\\textfiles1\\";
			xdirinfo di1(str1);

			CHECK_EQUAL(false, di1 != str1);
			CHECK_EQUAL(true, di1 != "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(non_equality_operator2)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(false, di1 != xdirpath(str1));
			CHECK_EQUAL(true, di1 != xdirpath("TEST:\\textfiles2\\"));
		}

		UNITTEST_TEST(non_equality_operator3)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(false, di1 != xdirinfo(str1));
			CHECK_EQUAL(true, di1 != xdirinfo("TEST:\\textfiles2\\"));
		}
	}
}
UNITTEST_SUITE_END
