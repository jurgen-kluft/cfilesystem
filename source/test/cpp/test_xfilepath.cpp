#include "cbase/c_target.h"
#include "cbase/c_runes.h"
#include "ctime/c_datetime.h"

#include "cunittest/cunittest.h"

#include "cfilesystem/private/c_filedevice.h"
#include "cfilesystem/c_filesystem.h"
#include "cfilesystem/c_filepath.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/c_stream.h"

using namespace ncore;

extern alloc_t* gTestAllocator;

UNITTEST_SUITE_BEGIN(filepath)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP()
		{
			filesystem_t::context_t ctxt;
			ctxt.m_allocator = gTestAllocator;
			ctxt.m_max_open_files = 32;
			filesystem_t::create(ctxt);
		}

		UNITTEST_FIXTURE_TEARDOWN()
		{
			filesystem_t::destroy();
		}

		UNITTEST_TEST(constructor1)
		{
			filepath_t p1;

			CHECK_TRUE(p1.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "TEST:\\textfiles\\docs\\readme.txt";
			filepath_t p = filesystem_t::filepath(str);

			CHECK_FALSE(p.isEmpty());
		}

	}
}
UNITTEST_SUITE_END
