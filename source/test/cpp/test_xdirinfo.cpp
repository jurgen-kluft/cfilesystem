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

extern xcore::alloc_t* gTestAllocator;

class utf32_alloc : public xcore::runes_alloc_t
{
public:
    virtual utf32::runes allocate(s32 len, s32 cap) 
	{
		utf32::prune prunes = (utf32::prune)gTestAllocator->allocate(sizeof(utf32::rune) * cap, sizeof(utf32::rune));
		prunes[cap - 1] = utf32::TERMINATOR;
		utf32::runes runes(prunes, prunes, prunes + cap - 1);
		return runes;
	}
            
	virtual void  deallocate(utf32::runes& slice_t)
	{
		if (slice_t.m_str != nullptr)
		{
			gTestAllocator->deallocate(slice_t.m_str);
		}
		slice_t.m_str = nullptr;
		slice_t.m_end = nullptr;
		slice_t.m_eos = nullptr;
	}
};
static utf32_alloc sUtf32Alloc;

UNITTEST_SUITE_BEGIN(dirinfo)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		static datetime_t sCreationTime(2011, 2, 10, 15, 30, 10);
		static datetime_t sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static datetime_t sLastWriteTime(2011, 2, 11, 10, 46, 20);


		UNITTEST_TEST(constructor0)
		{
			dirinfo_t di;
			CHECK_TRUE(di.getDirpath().isEmpty());
		}

		UNITTEST_TEST(constructor1)
		{
			const char* str = "TEST:\\textfiles\\docs";
			dirpath_t dp = filesystem_t::dirpath(str);
			dirinfo_t di ( dp );
			CHECK_TRUE(di.getDirpath() == dp);

			const char* str2 = "textfiles\\docs";
			dirpath_t dp2 = filesystem_t::dirpath(str2);
			dirinfo_t di2(dp2);
			CHECK_TRUE(di2.getDirpath() == dp2);
		}

	}
}
UNITTEST_SUITE_END
