#include "xbase\x_types.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string.h"
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
			char systemFilenameBuffer1[512];
			xcstring systemFilename1(systemFilenameBuffer1, sizeof(systemFilenameBuffer1));
			xfilesystem::createSystemPath("sdfsfsa:\\app.config", systemFilename1);

			char systemFilenameBuffer2[512];
			xcstring systemFilename2(systemFilenameBuffer2, sizeof(systemFilenameBuffer2));
			xfilesystem::createSystemPath("curdir:\\app.config", systemFilename2);

			CHECK_EQUAL(0, x_strCompare(systemFilename1.c_str(), systemFilename2.c_str()));
		}
	}
}
UNITTEST_SUITE_END

