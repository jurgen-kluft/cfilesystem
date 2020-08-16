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

		static xdatetime sCreationTime(2011, 2, 10, 15, 30, 10);
		static xdatetime sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static xdatetime sLastWriteTime(2011, 2, 11, 10, 46, 20);

		static ascii::rune sStringBuffer[512];
		static ascii::runes sCString(sStringBuffer, sStringBuffer, sStringBuffer + sizeof(sStringBuffer));

		UNITTEST_TEST(constructor1)
		{
			const char* filename = "TEST:\\readme.txt";
			xfilepath filepath = xfilesystem::filepath(filename);
			xfileinfo fi1(filepath);
			bool c = fi1.getFilepath() == xfilesystem::filepath(filename);
			CHECK_TRUE(c);
		}

		UNITTEST_TEST(constructor2)
		{
			const char* filename = "TEST:\\readme.txt";
			xfilepath fp1 = xfilesystem::filepath(filename);
			xfileinfo fi1(fp1);
			xfileinfo fi2(fi1);
			CHECK_TRUE(fi1.getFilepath() == fp1);
			CHECK_TRUE(fi2.getFilepath() == fp1);
			CHECK_TRUE(fi1 == fi2);
		}

		UNITTEST_TEST(sCopy)
		{
			const char* filename1 = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1 = xfilesystem::filepath(filename1);
			const char* filename2 = "Test:\\sCopy\\test.txt";
			xfilepath fp2 = xfilesystem::filepath(filename2);
			CHECK_TRUE(xfileinfo::sCopy(fp1,fp2,true));
			xfileinfo fi1(fp1);
			xfileinfo fi2(fp2);
			CHECK_EQUAL(fi1.getLength(),fi2.getLength());
			CHECK_TRUE(xfileinfo::sDelete(fp2));
			CHECK_FALSE(xfileinfo::sExists(fp2));
			CHECK_TRUE(xfileinfo::sExists(fp1));
		}

		UNITTEST_TEST(sMove)
		{
			const char* filename1 = "Test:\\readonly_files\\readme.txt";
			xfilepath fp1 = xfilesystem::filepath(filename1);
			const char* filename2 = "Test:\\sMove\\test.txt";
			xfilepath fp2 = xfilesystem::filepath(filename2);
			CHECK_TRUE(xfileinfo::sMove(fp1,fp2));
			CHECK_FALSE(xfileinfo::sExists(fp1));
		}
	}
}
UNITTEST_SUITE_END
