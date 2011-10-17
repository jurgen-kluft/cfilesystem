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
#include "xfilesystem\private\x_devicealias.h"
#include "xfilesystem\x_dirinfo.h"
#include "xfilesystem\private\x_filesystem_common.h"

using namespace xcore;
using namespace xfilesystem;

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

		UNITTEST_TEST(setLength)
		{
			const char* filename1 = "TEST:\\readonly_files\\readme.txt";
			xfileinfo fi1(filename1);
			CHECK_TRUE(fi1.getLength() == 48);
			fi1.setLength(100);
			CHECK_TRUE(fi1.getLength() == 100);
			fi1.setLength(48);
			CHECK_TRUE(fi1.getLength() == 48);
		}

		UNITTEST_TEST(isValid)
		{
			const char* filename = "TEST:\\readme.txt";
			xfileinfo fi1(filename);
			CHECK_TRUE(fi1.isValid());

			const char* filename2 = "INVALID:\\readme.txt";
			xfileinfo fi2(filename2);
			CHECK_FALSE(fi2.isValid());

			const char* filename3 = "test\\test.txt";
			xfileinfo fi3(filename3);
			CHECK_TRUE(fi3.isValid());
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

			xbyte buffer[8192];
			xcore::u64 numBytesRead = fi1.readAllBytes(buffer, 10);
			CHECK_EQUAL(10, numBytesRead);
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

		UNITTEST_TEST(copy)
		{
			const char* filenameSrc = "TEST:\\writeable_files\\file.txt";
			//xfilepath pathSrc(filenameSrc);
			xfileinfo fi1(filenameSrc);
			const char* filenameDes = "TEST:\\test\\test.txt";
			xfilepath pathDes(filenameDes);
			fi1.copy(pathDes,false);
			xfileinfo fi2(filenameDes);
			CHECK_EQUAL(fi2.getLength(),fi1.getLength());
		}

		UNITTEST_TEST(move)
		{
			const char* toFilename = "Test:\\move\\move_test.txt";
			xfilepath pathTo(toFilename);
			const char* filenameSrc = "Test:\\test\\test.txt";
			xfileinfo fi1(filenameSrc);
			fi1.move(pathTo);
			xfileinfo fi2(toFilename);
			xfileinfo fi3(filenameSrc);
			CHECK_EQUAL(fi1.getLength(),fi2.getLength());
			CHECK_EQUAL(fi3.getLength() ,-1);
		}

		UNITTEST_TEST(getAlias)
		{
			const char* filename1 = "Test:\\mov\\move_test.txt";
			xfileinfo fi1(filename1);
			const xdevicealias* alias = fi1.getAlias();
			CHECK_EQUAL(alias->alias(),"TEST");
		}

		UNITTEST_TEST(getPath)
		{
			const char* filename1 = "Test:\\move\\move_test.txt";
			const char* filename2 = "Test:\\move";
			xfileinfo fi1(filename1);
			xdirinfo path1(filename2);
			xdirinfo path2;
			fi1.getPath(path2); 
			CHECK_TRUE(path2 == path1 );
		}

		UNITTEST_TEST(getRoot)
		{
			const char* filename1 = "Test:\\move\\move_test.txt";
			const char* dirpath = "Test:\\";
			xfileinfo fi1(filename1);
			xdirinfo path1;
			fi1.getRoot(path1);
			xdirinfo path2(dirpath);
			CHECK_TRUE(path1 == path2);
		}

		UNITTEST_TEST(getParent)
		{
			const char* filename1 = "Test:\\move\\move_test.txt";
			const char* filename2 = "Test:\\";
			xfileinfo fi1(filename1);
			xdirinfo dir1;
			xdirinfo dir2(filename2);
			fi1.getParent(dir1);
			CHECK_TRUE(dir1 == dir2);
		}

		UNITTEST_TEST(getSubDir)
		{
			const char* filename1 = "Test:\\move\\move_test.txt";
			const char* subDir = "movable";
			const char* filename2 = "Test:\\move\\movable\\";
			xfileinfo fi1(filename1);
			xdirinfo di1;
			xdirinfo di2(filename2);
			fi1.getSubDir(subDir,di1);
			CHECK_TRUE(di1 == di2);
		}

		UNITTEST_TEST(onlyFilename)
		{
			const char* filename1 = "Test:\\move\\move_test.txt";
			const char* filename2 = "move_test.txt";
			xfileinfo fi1(filename1);
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi1.onlyFilename() == fi2);

			const char* filename3 = "Test:\\move";
			const char* filename4 = "move";
			xfileinfo fi3(filename3);
			xfileinfo fi4(filename4);
			CHECK_TRUE(fi3.onlyFilename() == fi4);
		}

		UNITTEST_TEST(getFilename)
		{
			const char* filename1 = "Test:\\move\\move_test.txt";
			const char* filename2 = "move_test.txt";
			xfileinfo fi1(filename1);
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi1.getFilename() == fi2);
		}

		UNITTEST_TEST(up)
		{
			const char* filename1 = "Test:\\move\\move_test.txt";
			const char* filename2 = "Test:\\move_test.txt";
			xfileinfo fi1(filename1);
			xfileinfo fi2(filename2);
			fi1.up();
			CHECK_TRUE(fi1 == fi2);
		}

		UNITTEST_TEST(down)
		{
			const char* filename1 = "Test:\\move_test.txt";
			const char* filename2 = "Test:\\move\\move_test.txt";
			xfileinfo fi1(filename1);
			xfileinfo fi2(filename2);
			fi1.down("move");
			CHECK_TRUE(fi1 == fi2);
		}

		UNITTEST_TEST(getTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime creationTime;
			xdatetime lastAccessTime;
			xdatetime lastWriteTime;
			fi1.getTime(creationTime,lastAccessTime,lastWriteTime);
			CHECK_TRUE(creationTime == xdatetime(2011, 2, 10, 15, 30, 10));
			CHECK_TRUE(lastAccessTime == xdatetime(2011, 2, 12, 16, 00, 20));
			CHECK_TRUE(lastWriteTime == xdatetime(2011, 2, 11, 10, 46, 20));
		}

		UNITTEST_TEST(getCreationTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime creationTime;
			fi1.getCreationTime(creationTime);
			CHECK_TRUE(creationTime == xdatetime(2011, 2, 10, 15, 30, 10));
		}

		UNITTEST_TEST(getLastAccessTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime lastAccessTime;
			fi1.getLastAccessTime(lastAccessTime);
			CHECK_TRUE(lastAccessTime == xdatetime(2011, 2, 12, 16, 00, 20));
		}

		UNITTEST_TEST(getLastWriteTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime lastWriteTime;
			fi1.getLastWriteTime(lastWriteTime);
			CHECK_TRUE(lastWriteTime == xdatetime(2011, 2, 11, 10, 46, 20));
		}

		UNITTEST_TEST(setTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime creationTime(2000,1,1,1,1,1);
			xdatetime lastAccessTime(2001,2,2,2,2,2);
			xdatetime lastWriteTime(2002,3,3,3,3,3);
			fi1.setTime(creationTime,lastAccessTime,lastWriteTime);

			xdatetime creationTime_Temp;
			xdatetime lastAccessTime_Temp;
			xdatetime lastWriteTime_Temp;
			fi1.getTime(creationTime_Temp,lastAccessTime_Temp,lastWriteTime_Temp);
			CHECK_TRUE(creationTime == creationTime_Temp);
			CHECK_TRUE(lastAccessTime == lastAccessTime_Temp);
			CHECK_TRUE(lastWriteTime == lastWriteTime_Temp);

			fi1.setTime(xdatetime(2011, 2, 10, 15, 30, 10),xdatetime(2011, 2, 12, 16, 00, 20),xdatetime(2011, 2, 11, 10, 46, 20));
		}

		UNITTEST_TEST(setCreationTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime creationTime;
			fi1.getCreationTime(creationTime);
			CHECK_TRUE(creationTime == xdatetime(2011, 2, 10, 15, 30, 10));

			fi1.setCreationTime(xdatetime(2000,1,1,1,1,1));
			xdatetime creationTime_Temp;
			fi1.getCreationTime(creationTime_Temp);
			CHECK_TRUE(creationTime_Temp == xdatetime(2000,1,1,1,1,1));
			fi1.setCreationTime(xdatetime(2011,2,10,15,30,10));
		}

		UNITTEST_TEST(setLastAccessTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime lastAccessTime;
			fi1.getLastAccessTime(lastAccessTime);
			CHECK_TRUE(lastAccessTime == xdatetime(2011, 2, 12, 16, 00, 20));

			fi1.setLastAccessTime(xdatetime(2001,2,2,2,2,2));
			xdatetime lastAccessTime_Temp;
			fi1.getLastAccessTime(lastAccessTime_Temp);
			CHECK_TRUE(lastAccessTime_Temp == xdatetime(2001,2,2,2,2,2));
			fi1.setLastAccessTime(xdatetime(2011, 2, 12, 16, 00, 20));
		}

		UNITTEST_TEST(setLastWriteTime)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xdatetime lastWriteTime;
			fi1.getLastWriteTime(lastWriteTime);
			CHECK_TRUE(lastWriteTime == xdatetime(2011, 2, 11, 10, 46, 20));

			fi1.setLastWriteTime(xdatetime(2002,3,3,3,3,3));
			xdatetime lastWriteTime_Temp;
			fi1.getLastWriteTime(lastWriteTime_Temp);
			CHECK_TRUE(lastWriteTime_Temp == xdatetime(2002,3,3,3,3,3));
			fi1.setLastWriteTime(xdatetime(2011, 2, 11, 10, 46, 20));
		}

		UNITTEST_TEST(assignment_operator1)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xfileinfo fi2(fi1);
			CHECK_TRUE(fi1.getFullName() == fi2.getFullName());
		}

		UNITTEST_TEST(assignment_operator2)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xfilepath fp1(filename1);
			xfileinfo fi2(fp1);
			CHECK_TRUE(fi1.getFullName() == fi2.getFullName());
		}

		UNITTEST_TEST(equal_operator)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xfileinfo fi2(filename1);
			CHECK_TRUE( fi1 == fi2);
		}

		UNITTEST_TEST(not_equal_operator)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfileinfo fi1(filename1);
			xfileinfo fi2(filename1);
			CHECK_FALSE(fi1 != fi2);
		}


		///< Static functions Unit tests
		UNITTEST_TEST(sExists)
		{
			const char* filename1 = "Test:\\writeable_files\\file.txt";
			xfilepath fp1(filename1);
			CHECK_TRUE(xfileinfo::sExists(fp1));
		}

		UNITTEST_TEST(sCreate)
		{
			const char* filename1 = "Test:\\sCreate\\test.txt";
			xfilepath fp1(filename1);
			xfilestream fs1;
			CHECK_FALSE(xfileinfo::sExists(fp1));
			CHECK_TRUE(xfileinfo::sCreate(fp1,fs1));
			CHECK_TRUE(xfileinfo::sExists(fp1));
		}

		UNITTEST_TEST(sDelete)
		{
			const char* file = "D:\\3.txt";
			xfilepath fp(file);
			xfilestream fs;
			xfileinfo::sDelete(fp);
			const char* filename1 = "Test:\\sCreate\\test.txt";
			xfilepath fp1(filename1);
			CHECK_TRUE(xfileinfo::sExists(fp1));
			CHECK_TRUE(xfileinfo::sDelete(fp1));
			CHECK_FALSE(xfileinfo::sExists(fp1));
		}

		UNITTEST_TEST(sGetLength)
		{
			const char* filename1 = "TEST:\\readme.txt";
			xfilepath fp1(filename1);
			CHECK_TRUE(xfileinfo::sGetLength(fp1) == -1);

			const char* filename2 = "TEST:\\readonly_files\\readme.txt";
			xfilepath fp2(filename2);
			CHECK_TRUE(xfileinfo::sGetLength(fp2) == 48);
		}

		UNITTEST_TEST(sSetLength)
		{
			const char* filename1 = "TEST:\\readonly_files\\readme.txt";
			xfilepath fp1(filename1);
			CHECK_TRUE(xfileinfo::sGetLength(fp1) == 48);
			xfileinfo::sSetLength(fp1,100);
			CHECK_TRUE(xfileinfo::sGetLength(fp1) == 100);
			xfileinfo::sSetLength(fp1,48);
			CHECK_TRUE(xfileinfo::sGetLength(fp1) == 48);
		}

		UNITTEST_TEST(sIsArchive)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfilepath fp1(filename1);
			CHECK_FALSE(xfileinfo::sIsArchive(fp1));

			const char* filename2 = "TEST:\\textfiles\\authors.txt";
			xfilepath fp2(filename2);
			CHECK_TRUE(xfileinfo::sIsArchive(fp2));
		}

		UNITTEST_TEST(sIsReadOnly)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfilepath fp1(filename1);
			CHECK_FALSE(xfileinfo::sIsReadOnly(fp1));

			const char* filename2 = "TEST:\\readonly_files\\readme.txt";
			xfilepath fp2(filename2);
			CHECK_TRUE(xfileinfo::sIsReadOnly(fp2));
		}

		UNITTEST_TEST(sIsHidden)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfileinfo fi1(filename1);
			CHECK_FALSE(fi1.isHidden());

			const char* filename2 = "TEST:\\textfiles\\docs\\tech.txt";
			xfileinfo fi2(filename2);
			CHECK_TRUE(fi2.isHidden());
		}

		UNITTEST_TEST(sIsSystem)
		{
			const char* filename1 = "TEST:\\textfiles\\readme1st.txt";
			xfilepath fp1(filename1);
			CHECK_FALSE(xfileinfo::sIsSystem(fp1));

			const char* filename2 = "TEST:\\textfiles\\tech\\install.txt";
			xfilepath fp2(filename2);
			CHECK_TRUE(xfileinfo::sIsSystem(fp2));
		}

		UNITTEST_TEST(sOpen)
		{
			const char* filename1 = "TEST:\\textfiles\\tech\\install.txt";
			xfilepath fp1(filename1);
			xfilestream fs1;
			CHECK_TRUE(xfileinfo::sOpen(fp1,fs1));
			CHECK_TRUE(fs1.isOpen());
			fs1.close();
		}

		UNITTEST_TEST(sOpenRead)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1(filename);
			xfilestream fs1;
			CHECK_TRUE(xfileinfo::sOpenRead(fp1,fs1));
			CHECK_TRUE(fs1.isOpen());
			fs1.close();
		}

		UNITTEST_TEST(sOpenWrite)
		{
			const char* filename = "TEST:\\writeable_files\\file.txt";
			xfilepath fp1(filename);
			xfilestream fs1;
			CHECK_TRUE(xfileinfo::sOpenWrite(fp1,fs1));
			CHECK_TRUE(fs1.isOpen());
			fs1.close();
		}

		UNITTEST_TEST(sReadAllBytes)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1(filename);

			xbyte buffer[8192];
			xcore::u64 numBytesRead = xfileinfo::sReadAllBytes(fp1,buffer,0,10);
			CHECK_EQUAL(10, numBytesRead);
		}

		UNITTEST_TEST(sWriteAllBytes)
		{
			const char* filename1 = "TEST:\\writeable_files\\file.txt";
			xfilepath fp1(filename1);

			xbyte buffer1[] = { "a unittest writing data to a file" };
			xcore::u64 numBytesWritten = xfileinfo::sWriteAllBytes(fp1,buffer1,0,sizeof(buffer1));
			CHECK_EQUAL(sizeof(buffer1), numBytesWritten);

			xbyte buffer2[] = { "a unittest writing data to a file with data much more big than the previous data size" };
			numBytesWritten = xfileinfo::sWriteAllBytes(fp1,buffer2,0,sizeof(buffer2));
			CHECK_EQUAL(sizeof(buffer2), numBytesWritten);

			xbyte buffer3[8192];
			xcore::u64 numBytesRead = xfileinfo::sReadAllBytes(fp1,buffer3,0,sizeof(buffer3));
			CHECK_EQUAL(numBytesWritten, numBytesRead);
		}

		UNITTEST_TEST(sSetTime_sGetTime)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1(filename);
			xdatetime createTime1,lastAccessTime1,lastWriteTime1;
			xfileinfo::sGetTime(fp1,createTime1,lastAccessTime1,lastWriteTime1);

			xdatetime createTime2(2000,1,1,1,1,1);
			xdatetime lastAccessTime2(2001,2,2,2,2,2);
			xdatetime lastWriteTime2(2002,3,3,3,3,3);
			xfileinfo::sSetTime(fp1,createTime2,lastAccessTime2,lastWriteTime2);

			xdatetime createTime3,lastAccessTime3,lastWriteTime3;
			xfileinfo::sGetTime(fp1,createTime3,lastAccessTime3,lastWriteTime3);
			CHECK_TRUE(createTime2 == createTime3);
			CHECK_TRUE(lastAccessTime2 == lastAccessTime3);
			CHECK_TRUE(lastWriteTime2 == lastWriteTime3);
			xfileinfo::sSetTime(fp1,createTime1,lastAccessTime1,lastWriteTime1);
		}

		UNITTEST_TEST(sSetCreationTime_sGetCreationTime)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1(filename);
			xdatetime createTime1;
			xfileinfo::sGetCreationTime(fp1,createTime1);

			xdatetime createTime2(2000,1,1,1,1,1);
			xfileinfo::sSetCreationTime(fp1,createTime2);

			xdatetime createTime3;
			xfileinfo::sGetCreationTime(fp1,createTime3);
			CHECK_TRUE(createTime3 == createTime2);
			xfileinfo::sSetCreationTime(fp1,createTime1);
		}

		UNITTEST_TEST(sSetLastAccessTime_sGetLastAccessTime)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1(filename);
			xdatetime lastAccessTime1;
			xfileinfo::sGetLastAccessTime(fp1,lastAccessTime1);

			xdatetime lastAccessTime2(2001,2,2,2,2,2);
			xfileinfo::sSetLastAccessTime(fp1,lastAccessTime2);

			xdatetime lastAccessTime3;
			xfileinfo::sGetLastAccessTime(fp1,lastAccessTime3);
			CHECK_TRUE(lastAccessTime3 == lastAccessTime2);
			xfileinfo::sSetLastAccessTime(fp1,lastAccessTime1);
		}

		UNITTEST_TEST(sSetLastWriteTime_sGetLastWriteTime)
		{
			const char* filename = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1(filename);
			xdatetime lastWriteTime1;
			xfileinfo::sGetLastWriteTime(fp1,lastWriteTime1);

			xdatetime lastWriteTime2(2001,2,2,2,2,2);
			xfileinfo::sSetLastWriteTime(fp1,lastWriteTime2);

			xdatetime lastWriteTime3;
			xfileinfo::sGetLastWriteTime(fp1,lastWriteTime3);
			CHECK_TRUE(lastWriteTime3 == lastWriteTime2);
			xfileinfo::sSetLastWriteTime(fp1,lastWriteTime1);
		}

		UNITTEST_TEST(sCopy)
		{
			const char* filename1 = "TEST:\\textfiles\\authors.txt";
			xfilepath fp1(filename1);
			const char* filename2 = "Test:\\sCopy\\test.txt";
			xfilepath fp2(filename2);
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
			xfilepath fp1(filename1);
			const char* filename2 = "Test:\\sMove\\test.txt";
			xfilepath fp2(filename2);
			CHECK_TRUE(xfileinfo::sMove(fp1,fp2));
			CHECK_FALSE(xfileinfo::sExists(fp1));
		}
	}
}
UNITTEST_SUITE_END
