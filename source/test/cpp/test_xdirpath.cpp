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

		static const char*		sFolders[] = {
			"the",
			"name",
			"is",
			"johhnywalker",
		};


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

			const char* str = "K:\\the\\name\\is\\johhnywalker";
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

		UNITTEST_TEST(getLevelOf1)
		{
			const char* str = "K:\\the\\name\\is\\johhnywalker";
			xdirpath di(str);
			
			CHECK_EQUAL(0, di.getLevelOf("the"));
			CHECK_EQUAL(1, di.getLevelOf("name"));
			CHECK_EQUAL(2, di.getLevelOf("is"));
			CHECK_EQUAL(3, di.getLevelOf("johhnywalker"));
			CHECK_EQUAL(-1, di.getLevelOf("johhny"));
		}

		UNITTEST_TEST(getLevelOf2)
		{
			const char* str1 = "K:\\the\\name\\is\\johhnywalker";
			xdirpath di1(str1);
			const char* str2 = "is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(2, di2.getLevelOf(di1));

			const char* str3 = "name\\is";
			xdirpath di3(str3);
			CHECK_EQUAL(-1, di3.getLevelOf(di1));
		}

		UNITTEST_TEST(isSubDirOf)
		{
			const char* str1 = "K:\\the\\name";
			xdirpath di1(str1);

			const char* str2 = "K:\\the\\name\\is\\johhnywalker";
			xdirpath di2(str2);
			CHECK_EQUAL(true, di2.isSubDirOf(di1));

			const char* str3 = "K:\\my\\name\\is\\johhnywalker";
			xdirpath di3(str3);
			CHECK_EQUAL(false, di3.isSubDirOf(di1));
		}
	}
}
UNITTEST_SUITE_END
