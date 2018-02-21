#include "xbase/x_types.h"
#include "xbase/x_string_std.h"

#include "xunittest/xunittest.h"

#include "xfilesystem/private/x_devicealias.h"
#include "xfilesystem/x_enumerator.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"

using namespace xcore;
using namespace xfilesystem;

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
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker\\";
			xdirpath dirpath1(str1);
			CHECK_EQUAL(false,dirpath1.isEmpty());
			CHECK_EQUAL(0,x_strCompareNoCase(str1,dirpath1.c_str()));
		}

		UNITTEST_TEST(constructor3)
		{
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker\\";
			xdirpath dirpath1(str1);
			CHECK_EQUAL(false,dirpath1.isEmpty());
			xdirpath dirpath_1(dirpath1);
			CHECK_EQUAL(0,x_strCompareNoCase(str1,dirpath_1.c_str()));

			xdirpath dirpath2;
			xdirpath dirpath_2(dirpath2);
			CHECK_EQUAL(true,dirpath_2.isEmpty());
		}

		UNITTEST_TEST(clear)
		{
			xdirpath di1;
			CHECK_EQUAL(true, di1.isEmpty());
			
			const char* str2 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(false, di2.isEmpty());
			di2.clear();
			CHECK_EQUAL(true, di2.isEmpty());
		}

		UNITTEST_TEST(getLength)
		{
			const char* str = "TEST:\\the\\name\\is\\johhnywalker\\";
			xdirpath di(str);

			CHECK_EQUAL(x_strlen(str), di.getLength());
			di.clear();
			CHECK_EQUAL(0, di.getLength());
		}

		UNITTEST_TEST(getMaxLength)
		{
			const char* str = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);
			CHECK_EQUAL(xdirpath::XDIRPATH_MAX, di.getMaxLength());

			xdirpath di2;
			CHECK_EQUAL(xdirpath::XDIRPATH_MAX,di2.getMaxLength());
		}

		UNITTEST_TEST(isEmpty)
		{
			xdirpath di1;
			CHECK_EQUAL(true, di1.isEmpty());

			const char* str2 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(false, di2.isEmpty());
			di2.clear();
			CHECK_EQUAL(true, di2.isEmpty());
		}

		UNITTEST_TEST(enumLevels)
		{
			struct enum_levels_test_enumerator : public enumerate_delegate<char>
			{
				bool ok;
				bool reversed;
				enum_levels_test_enumerator() : ok(true), reversed(false) {}
				virtual void operator () (s32 depth, const char* folder, bool& terminate)
				{
					const s32 numFolders = sizeof(sFolders) / sizeof(const char*);

					if (reversed)
						depth = numFolders-1 - depth;

					if (depth>=0 && depth<numFolders)
					{
						ok = x_strCompareNoCase(sFolders[depth], folder) == 0;
					}
					else
					{
						ok = false;
					}
					terminate = !ok;
				}
				virtual void operator() (s32 depth, const char& folder,bool& terminate) { }
			};

			const char* str = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);

			enum_levels_test_enumerator e;
			e.ok = false;
			e.reversed = false;
			di.enumLevels(e);
			CHECK_EQUAL(true, e.ok);

			e.ok = false;
			e.reversed = true;
			di.enumLevels(e, true);
			CHECK_EQUAL(true, e.ok);
		}

		UNITTEST_TEST(getLevels)
		{
			const char* str = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);
			CHECK_EQUAL(4, di.getLevels());

			xdirpath di2;
			CHECK_EQUAL(0, di2.getLevels());
		}

		UNITTEST_TEST(getLevelOf1)
		{
			const char* str = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);

			CHECK_EQUAL(0, di.getLevelOf("the"));
			CHECK_EQUAL(1, di.getLevelOf("name"));
			CHECK_EQUAL(2, di.getLevelOf("is"));
			CHECK_EQUAL(3, di.getLevelOf("johhnywalker"));
			CHECK_EQUAL(-1, di.getLevelOf("johhny"));
		}

		UNITTEST_TEST(getLevelOf2)
		{
// 			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker";
// 			xdirpath di1(str1);
// 			const char* str2 = "is\\johhnywalker";
// 			xdirpath di2(str2);
// 			CHECK_EQUAL(2, di2.getLevelOf(di1));
// 
// 			const char* str3 = "name\\is";
// 			xdirpath di3(str3);
// 			CHECK_EQUAL(-1, di3.getLevelOf(di1));
// 
// 			const char* str4 = "the\\name\\is\\johhnywalker\\next";
// 			xdirpath di4(str4);
// 			CHECK_EQUAL(0,di4.getLevelOf(di1));
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di1(str1);
			const char* str2 = "TEST:\\the\\name";
			xdirpath di2(str2);
			CHECK_EQUAL(2, di1.getLevelOf(di2));

			const char* str3 = "TEST:";
			xdirpath di3(str3);
			CHECK_EQUAL(4, di1.getLevelOf(di3));

			const char* str4 = "the\\name\\is\\johhnywalker\\next";
			xdirpath di4(str4);
			CHECK_EQUAL(-1,di4.getLevelOf(di1));

			const char* str5 = "C:\\the\\name\\is\\johhnywalker\\abc";
			xdirpath di5(str5);
			CHECK_EQUAL(-1,di5.getLevelOf(di1));

			const char* str6 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di6(str6);
			CHECK_EQUAL(0,di6.getLevelOf(di1));
		}

		UNITTEST_TEST(isRoot)
		{
			const char* str1 = "TEST:\\local\\folder\\";
			xdirpath di1(str1);
			CHECK_EQUAL(false, di1.isRoot());

			const char* str2 = "TEST:\\";
			xdirpath di2(str2);
			CHECK_EQUAL(true, di2.isRoot());
		}

		UNITTEST_TEST(isRooted)
		{
			const char* str1 = "local\\folder\\";
			xdirpath di1(str1);
			CHECK_EQUAL(false, di1.isRooted());

			const char* str2 = "TEST:\\local\\folder\\";
			xdirpath di2(str2);
			CHECK_EQUAL(true, di2.isRooted());
		}

		UNITTEST_TEST(isSubDirOf)
		{
			const char* str1 = "TEST:\\the\\name";
			xdirpath di1(str1);

			const char* str2 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(true, di2.isSubDirOf(di1));

			const char* str3 = "TEST:\\my\\name\\is\\johhnywalker";
			xdirpath di3(str3);
			CHECK_EQUAL(false, di3.isSubDirOf(di1));
		}

		UNITTEST_TEST(relative)
		{
			const char* str1 = "TEST:\\the\\name";
			xdirpath di1(str1);
			CHECK_EQUAL(true, di1.isRooted());

			xdirpath di2;
			di1.relative(di2);
			CHECK_EQUAL(false, di2.isRooted());
			CHECK_EQUAL(0, x_strCompare(di2.c_str(), "the\\name\\"));
		}

		UNITTEST_TEST(makeRelative)
		{
			const char* str1 = "TEST:\\the\\name";
			xdirpath di1(str1);
			CHECK_EQUAL(true, di1.isRooted());
			di1.makeRelative();
			CHECK_EQUAL(false, di1.isRooted());
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "the\\name\\"));

			const char* str2 = "the\\name";
			xdirpath di2(str2);
			CHECK_EQUAL(false, di2.isRooted());
			di2.makeRelative();
			CHECK_EQUAL(false, di2.isRooted());
			CHECK_EQUAL(0, x_strCompare(di2.c_str(), "the\\name\\"));
		}

		UNITTEST_TEST(makeRelativeTo)
		{
			const char* str1 = "the\\name\\folder";
			xdirpath di1(str1);
			CHECK_EQUAL(false, di1.isRooted());

			const char* str2 = "TEST:\\the\\name";
			xdirpath di2(str2);

			di1.makeRelativeTo(di2);
			CHECK_EQUAL(true, di1.isRooted());
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\the\\name\\folder\\"));

			const char* str3 = "a\\b\\c\\m";
			xdirpath di3(str3);
			CHECK_EQUAL(false, di3.isRooted());

			const char* str4 = "TEST:\\d\\e\\a\\b\\c\\d\\e\\a\\b\\c";
			xdirpath di4(str4);

			di3.makeRelativeTo(di4);
			CHECK_EQUAL(true, di3.isRooted());
			CHECK_EQUAL(0, x_strCompare(di3.c_str(), "TEST:\\d\\e\\a\\b\\c\\d\\e\\a\\b\\c\\m\\"));

			const char* str5 = "e\\f";
			xdirpath di5(str5);
			const char* str6 = "h\\i";
			xdirpath di6(str6);
			di5.makeRelativeTo(di6);
			CHECK_EQUAL(-1,x_strCompare(di5.c_str(),"h\\i\\e\\f\\"));
		}

		UNITTEST_TEST(up)
		{
			const char* str1 = "TEST:\\a\\b\\c\\d";
			xdirpath di1(str1);

			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\b\\c\\d\\"));
			di1.up();
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\b\\c\\"));
			di1.up();
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\b\\"));
			di1.up();
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\"));
			di1.up();
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\"));
			di1.up();
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\"));
		}

		UNITTEST_TEST(down)
		{
			const char* str1 = "TEST:\\";
			xdirpath di1(str1);

			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\"));

			di1.down("a");
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\"));
			di1.down("b");
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\b\\"));
			di1.down("c");
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\b\\c\\"));
			di1.down("d");
			CHECK_EQUAL(0, x_strCompare(di1.c_str(), "TEST:\\a\\b\\c\\d\\"));
		}

		UNITTEST_TEST(split)
		{
			const char* str = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);

			xdirpath parent;
			xdirpath sub;
			di.split(2, parent, sub);
			CHECK_EQUAL(0, x_strcmp(parent.c_str(), "TEST:\\the\\name\\"));
			CHECK_EQUAL(0, x_strcmp(sub.c_str(), "is\\johhnywalker\\"));

			di.split(1, parent, sub);
			CHECK_EQUAL(0, x_strcmp(parent.c_str(), "TEST:\\the\\"));
			CHECK_EQUAL(0, x_strcmp(sub.c_str(), "name\\is\\johhnywalker\\"));
		}

		UNITTEST_TEST(getName)
		{
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di1(str1);

			char strBuffer[64];
			xcstring str(strBuffer, sizeof(strBuffer));

			CHECK_EQUAL(true, di1.getName(str));
			CHECK_EQUAL(true, str == "johhnywalker");
		}

		UNITTEST_TEST(hasName)
		{
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di1(str1);

			CHECK_EQUAL(true, di1.hasName("the"));
			CHECK_EQUAL(true, di1.hasName("name"));
			CHECK_EQUAL(true, di1.hasName("is"));
			CHECK_EQUAL(true, di1.hasName("johhnywalker"));

			CHECK_EQUAL(false, di1.hasName("not"));
		}

		UNITTEST_TEST(getAlias)
		{
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di1(str1);

			CHECK_EQUAL(xdevicealias::sFind("TEST"), di1.getAlias());
			CHECK_EQUAL(xdevicealias::sFind("TEST"), di1.getAlias());

			const char* str2 = "INVALID:\\the\\name\\is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(xdevicealias::sFind("curdir"), di2.getAlias());

			const char* str3 = "the\\name\\is\\johhnywalker";
			xdirpath di3(str3);
			CHECK_EQUAL(xdevicealias::sFind("curdir"), di3.getAlias());
		}

		UNITTEST_TEST(getDevice)
		{
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di1(str1);

			CHECK_EQUAL(di1.getAlias()->device(), di1.getDevice());

			const char* str2 = "INVALID:\\the\\name\\is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(xdevicealias::sFind("curdir")->device(), di2.getDevice());

			const char* str3 = "the\\name\\is\\johhnywalker";
			xdirpath di3(str3);
			CHECK_EQUAL(xdevicealias::sFind("curdir")->device(), di3.getDevice());
		}

		UNITTEST_TEST(getSystem)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);
			const char* str2 = "TEST:/textfiles/";
			char strBuffer[256];
			xcstring str(strBuffer, sizeof(strBuffer));
			xfiledevice* device = di1.getSystem(str);
			CHECK_NOT_NULL(device);
#if defined(TARGET_PC) || defined(TARGET_360)
			CHECK_EQUAL(0, x_strCompare(str1, str.c_str()));
#endif
#if defined(TARGET_PS3) || defined(TARGET_PSP) || defined(TARGET_WII)
			CHECK_EQUAL(0,x_strCompare(str2,str.c_str()));
#endif

			x_printf("getSystem() in test xdirpath returned: %s\n", str.c_str());

			const char* str4 = "";
			xdirpath di3(str4);
			char strBuffer3[256];
			xcstring str5(strBuffer3,sizeof(strBuffer3));
			xfiledevice* device3 = di3.getSystem(str5);
			CHECK_EQUAL(0,x_strCompare(str5.c_str(),xdevicealias::sFind("curdir")->remap()));
		}

		UNITTEST_TEST(getRoot)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);

			xdirpath root;
			di1.getRoot(root);
			CHECK_EQUAL(true, root == "TEST:\\");
		}

		UNITTEST_TEST(getParent)
		{
			const char* str1 = "TEST:\\textfiles\\subdir\\";
			xdirpath di1(str1);

			xdirpath parent;
			di1.getParent(parent);
			CHECK_EQUAL(true, parent == "TEST:\\textfiles\\");

			const char* str2 = "test\\";
			xdirpath di2(str2);
			xdirpath parent2;
			di2.getParent(parent2);
			CHECK_EQUAL(true,parent2 == "test\\");
		}

		UNITTEST_TEST(getSubDir)
		{
			const char* str1 = "TEST:\\textfiles\\subdir\\";
			xdirpath di1(str1);

			xdirpath sub;
			CHECK_TRUE(di1.getSubDir("subdir2", sub));
			CHECK_EQUAL(true, sub == "TEST:\\textfiles\\subdir\\subdir2\\");
		}


		UNITTEST_TEST(get_set_DeviceName)
		{
			const char* str1 = "TEST:\\textfiles\\subdir\\";
			xdirpath di1(str1);

			char deviceStrBuffer[256];
			xcstring deviceStr(deviceStrBuffer, sizeof(deviceStrBuffer));

			CHECK_TRUE(di1.getDeviceName(deviceStr));
			CHECK_TRUE(deviceStr == "TEST");

			di1.setDeviceName("USB");

			CHECK_TRUE(di1.getDeviceName(deviceStr));
			CHECK_TRUE(deviceStr == "USB");
		}

		UNITTEST_TEST(get_set_DevicePart)
		{
			const char* str1 = "TEST:\\textfiles\\subdir\\";
			xdirpath di1(str1);

			char deviceStrBuffer[256];
			xcstring deviceStr(deviceStrBuffer, sizeof(deviceStrBuffer));

			CHECK_TRUE(di1.getDevicePart(deviceStr));
			CHECK_TRUE(deviceStr == "TEST:\\");

			di1.setDevicePart("USB:\\");

			CHECK_TRUE(di1.getDevicePart(deviceStr));
			CHECK_TRUE(deviceStr == "USB:\\");
		}

		UNITTEST_TEST(asignment_operator1)
		{
			const char* str1 = "TEST:\\textfiles\\subdir\\";
			xdirpath di1(str1);
			CHECK_TRUE(di1 == str1);

			const char* str2 = "TEST:\\binfiles\\tracks\\";
			di1 = str2;
			CHECK_TRUE(di1 == str2);
		}

		UNITTEST_TEST(asignment_operator2)
		{
			const char* str1 = "TEST:\\textfiles\\subdir\\";
			xdirpath di1(str1);
			CHECK_TRUE(di1 == str1);

			const char* str2 = "TEST:\\binfiles\\tracks\\";
			xdirpath di2(str2);
			di1 = di2;
			CHECK_TRUE(di1 == str2);
		}


		UNITTEST_TEST(add_asign_operator)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);
			CHECK_TRUE(di1 == str1);

			di1 += "subdir";
			const char* str2 = "TEST:\\textfiles\\subdir\\";
			CHECK_TRUE(di1 == str2);
		}

		UNITTEST_TEST(equal_operator1)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);
			bool equal1 = di1 == str1;
			CHECK_TRUE(equal1);

			const char* str2 = "TEST:\\textfiles\\subdir\\";
			di1 = str2;
			bool equal2 = di1 == str2;
			CHECK_TRUE(equal2);

			bool equal3 = di1 == "bogus";
			CHECK_FALSE(equal3);
		}

		UNITTEST_TEST(not_equal_operator1)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);
			bool equal1 = di1 != str1;
			CHECK_FALSE(equal1);

			const char* str2 = "TEST:\\textfiles\\subdir\\";
			di1 = str2;
			bool equal2 = di1 != str2;
			CHECK_FALSE(equal2);

			bool equal3 = di1 != "bogus";
			CHECK_TRUE(equal3);
		}

		UNITTEST_TEST(equal_operator2)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);
			bool equal1 = di1 == xdirpath(str1);
			CHECK_TRUE(equal1);

			const char* str2 = "TEST:\\textfiles\\subdir\\";
			di1 = str2;
			bool equal2 = di1 == xdirpath(str2);
			CHECK_TRUE(equal2);

			bool equal3 = di1 == xdirpath("bogus");
			CHECK_FALSE(equal3);
		}

		UNITTEST_TEST(not_equal_operator2)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);
			bool equal1 = di1 != xdirpath(str1);
			CHECK_FALSE(equal1);

			const char* str2 = "TEST:\\textfiles\\subdir\\";
			di1 = str2;
			bool equal2 = di1 != xdirpath(str2);
			CHECK_FALSE(equal2);

			bool equal3 = di1 != xdirpath("bogus");
			CHECK_TRUE(equal3);
		}

		UNITTEST_TEST(index_operator)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirpath di1(str1);

			CHECK_EQUAL('T', di1[0]);
			CHECK_EQUAL(':', di1[4]);
			CHECK_EQUAL('\\', di1[15]);
			CHECK_EQUAL('\0', di1[16]);
			CHECK_EQUAL('\0', di1[100]);
			CHECK_EQUAL('\0', di1[1000]);
		}

	}
}
UNITTEST_SUITE_END
