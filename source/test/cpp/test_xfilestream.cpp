#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_filestream.h"
#include "xfilesystem\x_istream.h"
#include "xfilesystem\x_threading.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\x_fileinfo.h"

using namespace xcore;

	using namespace xfilesystem;


UNITTEST_SUITE_BEGIN(filestream)
{
	UNITTEST_FIXTURE(main)
	{
		void callbackWrite_TEST_func(x_asyncio_result ioResult){}
		void callbackRead_TEST_func(x_asyncio_result ioResult){}

		x_asyncio_callback_struct callbackWrite_TEST;
		x_asyncio_callback_struct callbackRead_TEST;

		UNITTEST_FIXTURE_SETUP() 
		{
			callbackWrite_TEST.callback = callbackWrite_TEST_func;
			callbackWrite_TEST.userData = NULL;

			callbackRead_TEST.callback = callbackRead_TEST_func;
			callbackRead_TEST.userData = NULL;
		}
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

		UNITTEST_TEST(construct1)
		{
			xfilestream xfs1;
			xfilestream xfs2;
			CHECK_EQUAL(xfs1.canRead(),xfs2.canRead());
			CHECK_EQUAL(xfs1.getLength(),xfs2.getLength());
		}

		UNITTEST_TEST(construct2)
		{
			xfilestream xfs1;
			xfilestream xfs2(xfs1);
			CHECK_EQUAL(xfs1.canRead(),xfs2.canRead());
			CHECK_EQUAL(xfs1.getLength(),xfs2.getLength());
		}

		UNITTEST_TEST(construct3)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.getLength();
			xfilestream xfs2(xfp1,FileMode_Create,FileAccess_ReadWrite,FileOp_Sync);
			u64 len2= xfs2.getLength();
			CHECK_EQUAL(xfs2.getLength(),0);
			xfs2.setLength(len1);
			CHECK_EQUAL(len1,xfs2.getLength());
		}

		UNITTEST_TEST(canRead)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.canRead(),true);
		}

		UNITTEST_TEST(canSeek)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.canSeek(),true);
		}

		UNITTEST_TEST(canWrite)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.canWrite(),true);
			xfilestream xfs2(xfp1,FileMode_Open,FileAccess_Read,FileOp_Sync);
			CHECK_EQUAL(xfs2.canWrite(),false);
		}

		UNITTEST_TEST(isOpen)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.isOpen(),true);

			const char* str2 = "TEST:\\test\\test\\test.txt";
			xfilepath xfp2(str2);
			xfilestream xfs2(xfp2,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs2.isOpen(),false); 
		}

		UNITTEST_TEST(isAsync)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.isAsync(),false);

			xfilestream xfs2(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Async, callbackRead_TEST);
			CHECK_EQUAL(xfs2.isAsync(),true);

		}

		UNITTEST_TEST(getLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.getLength();
			xfilestream xfs2(xfp1,FileMode_Create,FileAccess_ReadWrite,FileOp_Sync);
			u64 len2= xfs2.getLength();
			CHECK_EQUAL(xfs2.getLength(),0);
			xfs2.setLength(10);
			CHECK_EQUAL(10,xfs2.getLength());
			xfs2.setLength(len1);
		}

		UNITTEST_TEST(setLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.getLength();
			xfilestream xfs2(xfp1,FileMode_Create,FileAccess_ReadWrite,FileOp_Sync);
			u64 len2= xfs2.getLength();
			CHECK_EQUAL(xfs2.getLength(),0);
			xfs2.setLength(20);
			CHECK_EQUAL(20,xfs2.getLength());
			xfs2.setLength(len1);
		}

		UNITTEST_TEST(getPosition)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 pos1 = xfs1.getPosition();
			CHECK_EQUAL(pos1,0);
		}

		UNITTEST_TEST(setPosition)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xfs1.setPosition(10);
			CHECK_EQUAL(xfs1.getPosition(),10);
			xfs1.setPosition(0);
			CHECK_EQUAL(xfs1.getPosition(),0);
		}

		UNITTEST_TEST(seek)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.getPosition(),0);
			xfs1.seek(10,Seek_Begin);
			CHECK_EQUAL(xfs1.getPosition(),10);
			xfs1.seek(-10,Seek_Current);
			CHECK_EQUAL(xfs1.getPosition(),0);
		}

		UNITTEST_TEST(close)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xfs1.close();
			CHECK_EQUAL(xfs1.isOpen(),false);
		}

		UNITTEST_TEST(read)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.getPosition(),0);
			xbyte buffer1[8124];
			//			memset(buffer1,0,sizeof(buffer1));
			u64 fileLen1 = xfs1.read(buffer1,1,10); 
			CHECK_EQUAL(10,fileLen1);
		}

		UNITTEST_TEST(readByte)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte byte;
			u64 len = xfs1.readByte(byte);
			CHECK_EQUAL(byte,'T');
			CHECK_EQUAL(len,1);
		}

		UNITTEST_TEST(write)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.getPosition(),0);
			xbyte buffer1[10];
			u64 fileLen1 = xfs1.read(buffer1,0,10);
			CHECK_EQUAL(fileLen1,10);
			xbyte buffer_write[10] = "abcdefghi";
			xfs1.write(buffer_write,0,10);

			xbyte buffer_read[10];
			u64 fileLen3 = xfs1.read(buffer_read,0,10);
			CHECK_EQUAL(fileLen3,10);
			for(int n = 0; n<10;++n)
			{
				CHECK_EQUAL(buffer_write[n],buffer_read[n]);
			}
			xfs1.write(buffer1,0,10);
		}

		UNITTEST_TEST(writeByte)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.writeByte('M');
			xbyte byte;
			u64 len2 = xfs1.readByte(byte);
			CHECK_EQUAL(byte,'M');
			CHECK_EQUAL(len2,len1);
			xfs1.writeByte('T');
		}

		UNITTEST_TEST(beginRead_endRead)
		{
			// TODO rework this later
			/*
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_1 = &event_factory_temp;
			setEventFactory(event_factory_1);
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer1[8124];
			result1 = xfs1.beginRead(buffer1,8100,10,callbackRead_TEST);
			xfs1.endRead(result1);
			setEventFactory(NULL);
			*/
		}

		UNITTEST_TEST(beginWrite_endWrite)
		{
			// todo rework this later
			/*
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_1 = &event_factory_temp;
			setEventFactory(event_factory_1);
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.getPosition(),0);
			xbyte buffer1[10];
			u64 fileLen1 = xfs1.read(buffer1,0,10);
			CHECK_EQUAL(fileLen1,10);
			xbyte buffer_write[10] = "abcdefghi";
			xasync_result result1 = xfs1.beginWrite(buffer_write,0,10,callbackWrite_TEST);
			xfs1.endWrite(result1);
			xbyte buffer_read[10];
			u64 fileLen3 = xfs1.read(buffer_read,0,10);
			CHECK_EQUAL(fileLen3,10);
			for(int n = 0; n<10;++n)
			{
				CHECK_EQUAL(buffer_write[n],buffer_read[n]);
			}
			xasync_result result2 = xfs1.beginWrite(buffer1,0,10,callbackWrite_TEST);
			xfs1.endWrite(result2);
			setEventFactory(NULL);
			*/
		}

		UNITTEST_TEST(copyTo1)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer1[10];
			xfs1.read(buffer1,0,10);
			const char* str2 = "Test:\\copyTo1\\copy.txt";
			xfilepath xfp2(str2);
			xfilestream fpTemp;
			CHECK_FALSE(xfileinfo::sExists(xfp2));
			CHECK_TRUE(xfileinfo::sCreate(xfp2,fpTemp));
			CHECK_TRUE(xfileinfo::sExists(xfp2));
			xfilestream xfs2(xfp2,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xfs1.copyTo(xfs2);
			xbyte buffer2[10];
			xfs2.read(buffer2,0,10);
			for(int n =0; n < 10; ++n)
			{
				CHECK_EQUAL(buffer1[n],buffer2[n]);
			}
			CHECK_TRUE(xfileinfo::sDelete(xfp2));
			CHECK_FALSE(xfileinfo::sExists(xfp2));
		}

		UNITTEST_TEST(copyTo2)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer1[10];
			xfs1.read(buffer1,0,10);
			const char* str2 = "Test:\\copyTo1\\copy.txt";
			xfilepath xfp2(str2);
			xfilestream fpTemp;
			CHECK_FALSE(xfileinfo::sExists(xfp2));
			CHECK_TRUE(xfileinfo::sCreate(xfp2,fpTemp));
			CHECK_TRUE(xfileinfo::sExists(xfp2));
			xfilestream xfs2(xfp2,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xfs1.copyTo(xfs2,10);
			xbyte buffer2[20];
			u64 readLen = xfs2.read(buffer2,0,20);
			CHECK_EQUAL(readLen,10);
			for(int n = 0; n < 10; ++n)
			{
				CHECK_EQUAL(buffer1[n],buffer2[n]);
			}
			CHECK_TRUE(xfileinfo::sDelete(xfp2));
			CHECK_FALSE(xfileinfo::sExists(xfp2));
		}
	}
}
UNITTEST_SUITE_END
