#include "xbase\x_types.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_enumerator.h"
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

		static const char*		sFolders[] = {
			"the",
			"name",
			"is",
			"johhnywalker",
		};

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

		UNITTEST_TEST(sMaxLength)
		{
			CHECK_EQUAL(xdirpath::XDIR_MAX_PATH-2, xdirpath::sMaxLength());
		}

		UNITTEST_TEST(getMaxLength)
		{
			const char* str = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);
			CHECK_EQUAL(xdirpath::sMaxLength(), di.getMaxLength());
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
			struct enum_levels_test_enumerator : public enumerate_delegate<const char*>
			{
				bool ok;
				bool reversed;
				enum_levels_test_enumerator() : ok(true), reversed(false) {}
				virtual void operator () (s32 depth, const char* const& folder, bool& terminate)
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
			const char* str1 = "TEST:\\the\\name\\is\\johhnywalker";
			xdirpath di1(str1);
			const char* str2 = "is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(2, di2.getLevelOf(di1));

			const char* str3 = "name\\is";
			xdirpath di3(str3);
			CHECK_EQUAL(-1, di3.getLevelOf(di1));
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
	}
}
UNITTEST_SUITE_END
