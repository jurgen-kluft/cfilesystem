#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"

#include "xunittest/xunittest.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_stream.h"

using namespace xcore;

extern alloc_t* gTestAllocator;

UNITTEST_SUITE_BEGIN(dirpath)
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


		static const char*		sFolders[] = {
			"the",
			"name",
			"is",
			"johhnywalker",
		};

		UNITTEST_TEST(constructor1)
		{
			dirpath_t dirpath;
			CHECK_EQUAL(true,dirpath.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
			dirpath_t dirpath = filesystem_t::dirpath("c:\\the\\name\\is\\johhnywalker\\");
			CHECK_EQUAL(false, dirpath.isEmpty());
		}

		UNITTEST_TEST(to_string)
		{
			const char* asciidirstr = "c:\\the\\name\\is\\johhnywalker\\";
			dirpath_t dirpath = filesystem_t::dirpath(asciidirstr);
			runez_t<ascii::rune, 128> dirstr;
			dirpath.to_string(dirstr);
			CHECK_EQUAL(0, compare(dirstr, asciidirstr));
		}
	}
}
UNITTEST_SUITE_END
