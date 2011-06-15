#include "xbase\x_types.h"
#include "xbase\x_string_std.h"
#include "xtime\x_datetime.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\x_fileinfo.h"
#include "xfilesystem\x_filestream.h"
#include "xfilesystem\x_devicealias.h"

using namespace xcore;
using namespace xfilesystem;

UNITTEST_SUITE_BEGIN(fileinfo)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		static xdatetime sCreationTime(2011, 2, 10, 15, 30, 10);
		static xdatetime sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static xdatetime sLastWriteTime(2011, 2, 11, 10, 46, 20);

		static char sStringBuffer[512];
		static xcstring sCString(sStringBuffer, sizeof(sStringBuffer));

		UNITTEST_TEST(constructor1)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			CHECK_EQUAL(0, x_strCompare(fi1.getFullName().c_str(), filename));
		}

		UNITTEST_TEST(constructor2)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			xfileinfo fi2(fi1);
			CHECK_EQUAL(0, x_strCompare(fi1.getFullName().c_str(), filename));
			CHECK_EQUAL(0, x_strCompare(fi2.getFullName().c_str(), filename));
			CHECK_TRUE(fi1 == fi2);
		}

		UNITTEST_TEST(constructor3)
		{
			const char* filename = "TEST:\\readme.txt";
			xfilepath fp1(filename);
			xfileinfo fi2(fp1);
			CHECK_EQUAL(0, x_strCompare(fp1.c_str(), filename));
			CHECK_EQUAL(0, x_strCompare(fi2.getFullName().c_str(), filename));
			CHECK_TRUE(fp1 == fi2.getFullName());
		}

		UNITTEST_TEST(getFullName)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			xfilepath fp1(filename);
			CHECK_TRUE(fi1.getFullName() == fp1);
		}

		UNITTEST_TEST(getName)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			fi1.getName(sCString);			
			CHECK_TRUE(sCString == "readme");
		}

		UNITTEST_TEST(getExtension)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			fi1.getExtension(sCString);			
			CHECK_TRUE(sCString == ".txt");
		}

		UNITTEST_TEST(getLength)
		{
			const char* filename1 = "TEST:\\readme.txt";
			xfileinfo fi1(filename1);
			CHECK_TRUE(fi1.getLength() == -1);

			const char* filename2 = "TEST:\\readonly_files\\readme.txt";
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi2.getLength() == 48);
		}

		UNITTEST_TEST(isValid)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			CHECK_TRUE(fi1.isValid());

			const char* filename2 = "INVALID:\\readme.txt";
			xfileinfo fi2(filename2);
			CHECK_FALSE(fi2.isValid());
		}

		UNITTEST_TEST(isRooted)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			CHECK_TRUE(fi1.isRooted());

			const char* filename2 = "Rootless\\readme.txt";
			xfileinfo fi2(filename2);
			CHECK_FALSE(fi2.isRooted());
		}

		UNITTEST_TEST(isArchive)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfileinfo fi1(filename1);
			CHECK_FALSE(fi1.isArchive());

			const char* filename2 = "TEST:\\textfiles\\authors.txt";
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi2.isArchive());
		}

		UNITTEST_TEST(isReadOnly)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfileinfo fi1(filename1);
			CHECK_FALSE(fi1.isReadOnly());

			const char* filename2 = "TEST:\\readonly_files\\readme.txt";
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi2.isReadOnly());
		}

		UNITTEST_TEST(isHidden)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfileinfo fi1(filename1);
			CHECK_FALSE(fi1.isHidden());

			const char* filename2 = "TEST:\\textfiles\\docs\\tech.txt";
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi2.isHidden());
		}

		UNITTEST_TEST(isSystem)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfileinfo fi1(filename1);
			CHECK_FALSE(fi1.isSystem());

			const char* filename2 = "TEST:\\textfiles\\tech\\install.txt";
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi2.isSystem());
		}

		UNITTEST_TEST(exists)
		{
			const char* filename = "TEST:\\textfiles\\readme1st.txt";
			xfileinfo fi1(filename);
			CHECK_TRUE(fi1.exists());

			const char* filename2 = "TEST:\\textfiles\\does_not_exist.txt";
			xfileinfo fi2(filename2);
			CHECK_FALSE(fi2.exists());
		}

		UNITTEST_TEST(create1)
		{
			const char* filename = "TEST:\\new_textfile.txt";
			xfileinfo fi1(filename);
			CHECK_TRUE(fi1.create());
			CHECK_TRUE(fi1.remove());
		}

		UNITTEST_TEST(create2)
		{
			const char* filename = "TEST:\\new_textfile.txt";
			xfileinfo fi1(filename);
			
			xfilestream fs;
			CHECK_TRUE(fi1.create(fs));
			CHECK_TRUE(fi1.remove());
		}

		UNITTEST_TEST(remove)
		{
			const char* filename = "TEST:\\new_textfile.txt";
			xfileinfo fi1(filename);
			fi1.create();
			CHECK_TRUE(fi1.remove());
		}

		UNITTEST_TEST(openRead)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfileinfo fi1(filename);

			xfilestream fs;
			CHECK_TRUE(fi1.openRead(fs));
			CHECK_TRUE(fs.isOpen());

			fs.close();
		}

		UNITTEST_TEST(openWrite)
		{
			const char* filename = "TEST:\\writeable_files\\file.txt";
			xfileinfo fi1(filename);

			xfilestream fs;
			CHECK_TRUE(fi1.openWrite(fs));
			CHECK_TRUE(fs.isOpen());

			fs.close();
		}

		UNITTEST_TEST(readAllBytes)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfileinfo fi1(filename);
			xcore::u64 l = fi1.getLength();

			xbyte buffer[8192];
			xcore::u64 count = sizeof(buffer);
			xcore::u64 numBytesRead = fi1.readAllBytes(buffer, count);
			CHECK_EQUAL(l, numBytesRead);
		}

		UNITTEST_TEST(writeAllBytes)
		{
			const char* filename1 = "TEST:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);

			xbyte buffer1[] = { "a unittest writing data to a file" };
			xcore::u64 numBytesWritten = fi1.writeAllBytes(buffer1, sizeof(buffer1));
			CHECK_EQUAL(sizeof(buffer1), numBytesWritten);

			xbyte buffer2[] = { "a unittest writing data to a file with data bigger than the previous data size" };
			numBytesWritten = fi1.writeAllBytes(buffer2, sizeof(buffer2));
			CHECK_EQUAL(sizeof(buffer2), numBytesWritten);
			CHECK_EQUAL(sizeof(buffer2), fi1.getLength());

			xbyte buffer3[8192];
			xcore::u64 numBytesRead = fi1.readAllBytes(buffer3, sizeof(buffer3));
			CHECK_EQUAL(numBytesWritten, numBytesRead);
		}
	}
}
UNITTEST_SUITE_END
