#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"

#include "xunittest/xunittest.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_stream.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_path.h"

using namespace xcore;

extern xcore::xalloc* gTestAllocator;

class utf32_alloc : public xcore::utf32::alloc
{
public:
    virtual utf32::runes allocate(s32 len, s32 cap) 
	{
		utf32::prune prunes = (utf32::prune)gTestAllocator->allocate(sizeof(utf32::rune) * cap, sizeof(utf32::rune));
		prunes[cap - 1] = utf32::TERMINATOR;
		utf32::runes runes(prunes, prunes, prunes + cap - 1);
		return runes;
	}
            
	virtual void  deallocate(utf32::runes& slice)
	{
		if (slice.m_str != nullptr)
		{
			gTestAllocator->deallocate(slice.m_str);
		}
		slice.m_str = nullptr;
		slice.m_end = nullptr;
		slice.m_eos = nullptr;
	}
};
static utf32_alloc sUtf32Alloc;

UNITTEST_SUITE_BEGIN(dirinfo)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		static xdatetime sCreationTime(2011, 2, 10, 15, 30, 10);
		static xdatetime sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static xdatetime sLastWriteTime(2011, 2, 11, 10, 46, 20);


		UNITTEST_TEST(constructor0)
		{
			xdirinfo di;
			CHECK_TRUE(di.getDirpath().isEmpty());
		}

		UNITTEST_TEST(constructor1)
		{
			const char* str = "TEST:\\textfiles\\docs";
			xdirinfo di = xpath::as_dirinfo(str, &sUtf32Alloc);
			CHECK_EQUAL(0, xpath::cmp(di, str));

			const char* str2 = "textfiles\\docs";
			xdirinfo di2= xpath::as_dirinfo(str2, &sUtf32Alloc);
			CHECK_EQUAL(0, xpath::cmp(di2, str2));
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di = xpath::as_dirinfo(str, &sUtf32Alloc);
			xdirinfo di2(di);
			CHECK_EQUAL(0, xpath::cmp(di2, str));
		}

		UNITTEST_TEST(constructor3)
		{
			const char* str2 = "TEST:\\textfiles\\docs\\the_folder\\";
			xdirpath dp2 = xpath::as_dirpath(str2, &sUtf32Alloc);
			CHECK_EQUAL(0, xpath::cmp(dp2, str2));

			xdirinfo di1(dp2);
			CHECK_EQUAL(0, xpath::cmp(di1, str2));
		}

		UNITTEST_TEST(getFullName)
		{
			const char* str = "TEST:\\textfiles\\docs\\the_folder\\";
			xdirinfo di = xpath::as_dirinfo(str, &sUtf32Alloc);
			CHECK_EQUAL(0, xpath::cmp(di.getDirpath(), str));
		}

		UNITTEST_TEST(getName)
		{
			const char* str = "TEST:\\textfiles\\docs\\the_folder\\";
			xdirinfo di = xpath::as_dirinfo(str, &sUtf32Alloc);

			xdirpath name;
			di.getDirpath().getName(name);

			CHECK_EQUAL(0, xpath::cmp(name, "the_folder\\"));
		}

	}
}
UNITTEST_SUITE_END
