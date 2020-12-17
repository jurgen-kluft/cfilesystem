#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"

#include "xunittest/xunittest.h"

#include "filesystem_t/private/x_filedevice.h"
#include "filesystem_t/x_filesystem.h"
#include "filesystem_t/x_filepath.h"
#include "filesystem_t/x_dirpath.h"
#include "filesystem_t/x_dirinfo.h"
#include "filesystem_t/x_fileinfo.h"
#include "filesystem_t/x_stream.h"

using namespace xcore;


UNITTEST_SUITE_BEGIN(fileinfo)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() 
		{

			
		}
		UNITTEST_FIXTURE_TEARDOWN() 
		{

		}

		static datetime_t sCreationTime(2011, 2, 10, 15, 30, 10);
		static datetime_t sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static datetime_t sLastWriteTime(2011, 2, 11, 10, 46, 20);

		static ascii::rune sStringBuffer[512];
		static ascii::runes sCString(sStringBuffer, sStringBuffer, sStringBuffer + sizeof(sStringBuffer));

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
			const char* filename2 = "Test:\\sCopy\\test.txt";
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
			const char* filename1 = "Test:\\readonly_files\\readme.txt";
			filepath_t fp1 = filesystem_t::filepath(filename1);
			const char* filename2 = "Test:\\sMove\\test.txt";
			filepath_t fp2 = filesystem_t::filepath(filename2);
			CHECK_TRUE(fileinfo_t::sMove(fp1,fp2));
			CHECK_FALSE(fileinfo_t::sExists(fp1));
		}
	}
}
UNITTEST_SUITE_END
