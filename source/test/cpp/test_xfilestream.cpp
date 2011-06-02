#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_filestream.h"

using namespace xcore;
using namespace xfilesystem;



UNITTEST_SUITE_BEGIN(filestream)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(open)
		{
			xfilepath fp("TEST:\\textfiles\\readme1st.txt");
			xfilestream fs(fp, FileMode_Open, FileAccess_Read, FileOp_Sync);
			CHECK_TRUE(fs.isOpen());
			if (fs.isOpen())
			{
				fs.close();
				CHECK_FALSE(fs.isOpen());
			}
		}

	}
}
UNITTEST_SUITE_END
