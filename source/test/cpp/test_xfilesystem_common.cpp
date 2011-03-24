#include "xbase\x_types.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\x_threading.h"

using namespace xcore;

	
UNITTEST_SUITE_BEGIN(filesystem_common)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		// main 
		UNITTEST_TEST(createSystemPath)
		{
			char systemFilename1[512];
			xfilesystem::createSystemPath("sdfsfsa:\\app.config", systemFilename1, sizeof(systemFilename1) - 1);
			char systemFilename2[512];
			xfilesystem::createSystemPath("curdir:\\app.config", systemFilename2, sizeof(systemFilename2) - 1);

			CHECK_EQUAL(0, x_strCompare(systemFilename1, systemFilename2));
		}
	}
}
UNITTEST_SUITE_END

