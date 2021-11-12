#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"

#include "xunittest/xunittest.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_stream.h"

using namespace xcore;

extern alloc_t* gTestAllocator;
extern filedevice_t* sTestFileDevice;

UNITTEST_SUITE_BEGIN(fileinfo)
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

		static datetime_t sCreationTime(2011, 2, 10, 15, 30, 10);
		static datetime_t sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static datetime_t sLastWriteTime(2011, 2, 11, 10, 46, 20);

		static ascii::rune sStringBuffer[512];
		static runes_t sCString(sStringBuffer, sStringBuffer, sStringBuffer + sizeof(sStringBuffer));

		UNITTEST_TEST(register_test_filedevice)
		{
			runez_t<utf32::rune, 32> deviceName;
			deviceName = "TEST:\\";
			CHECK_TRUE(filesystem_t::register_device(deviceName, sTestFileDevice));
		}

		UNITTEST_TEST(constructor1)
		{
			const char* filename = "TEST:\\readme.txt";
			filepath_t filepath = filesystem_t::filepath(filename);
			fileinfo_t fi1(filepath);
			bool c = fi1.getFilepath() == filesystem_t::filepath(filename);
			CHECK_TRUE(c);
		}

		UNITTEST_TEST(constructor2)
		{
			const char* filename = "TEST:\\readme.txt";
			filepath_t fp1 = filesystem_t::filepath(filename);
			fileinfo_t fi1(fp1);
			fileinfo_t fi2(fi1);
			CHECK_TRUE(fi1.getFilepath() == fp1);
			CHECK_TRUE(fi2.getFilepath() == fp1);
			CHECK_TRUE(fi1 == fi2);
		}

		UNITTEST_TEST(sCopy)
		{
			const char* filename1 = "TEST:\\textfiles\\authors.txt";
			filepath_t fp1 = filesystem_t::filepath(filename1);
			const char* filename2 = "TEST:\\sCopy\\test.txt";
			filepath_t fp2 = filesystem_t::filepath(filename2);
			CHECK_TRUE(fileinfo_t::sCopy(fp1,fp2,true));
			fileinfo_t fi1(fp1);
			fileinfo_t fi2(fp2);
			CHECK_EQUAL(fi1.getLength(),fi2.getLength());
			CHECK_TRUE(fileinfo_t::sDelete(fp2));
			CHECK_FALSE(fileinfo_t::sExists(fp2));
			CHECK_TRUE(fileinfo_t::sExists(fp1));
		}

		UNITTEST_TEST(sMove)
		{
			const char* filename1 = "TEST:\\readonly_files\\readme.txt";
			filepath_t fp1 = filesystem_t::filepath(filename1);
			const char* filename2 = "TEST:\\sMove\\test.txt";
			filepath_t fp2 = filesystem_t::filepath(filename2);
			CHECK_TRUE(fileinfo_t::sMove(fp1,fp2));
			CHECK_FALSE(fileinfo_t::sExists(fp1));
		}
	}
}
UNITTEST_SUITE_END
