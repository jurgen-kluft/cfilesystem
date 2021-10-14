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

UNITTEST_SUITE_BEGIN(filestream)
{
	UNITTEST_FIXTURE(main)
	{
//		void callbackWrite_TEST_func(x_asyncio_result ioResult){}
//		void callbackRead_TEST_func(x_asyncio_result ioResult){}
//
//		x_asyncio_callback_struct callbackWrite_TEST;
//		x_asyncio_callback_struct callbackRead_TEST;

		UNITTEST_FIXTURE_SETUP()
		{
			filesystem_t::context_t ctxt;
			ctxt.m_allocator = gTestAllocator;
			ctxt.m_max_open_files = 32;
			filesystem_t::create(ctxt);
			// callbackWrite_TEST.callback = callbackWrite_TEST_func;
			// callbackWrite_TEST.userData = NULL;
			// 
			// callbackRead_TEST.callback = callbackRead_TEST_func;
			// callbackRead_TEST.userData = NULL;
		}

		UNITTEST_FIXTURE_TEARDOWN()
		{
			filesystem_t::destroy();
		}

		UNITTEST_TEST(open)
		{
			filepath_t fp = filesystem_t::filepath("TEST:\\textfiles\\readme1st.txt");
			stream_t fs = filesystem_t::open(fp, FileMode_Open, FileAccess_Read, FileOp_Sync);
			CHECK_TRUE(fs.isOpen());
			if (fs.isOpen())
			{
				fs.close();
				CHECK_FALSE(fs.isOpen());
			}
		}

		UNITTEST_TEST(construct1)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_Read,FileOp_Sync);
			stream_t xfs2 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_Read,FileOp_Sync);
			CHECK_EQUAL(xfs1.canRead(),xfs2.canRead());
			CHECK_EQUAL(xfs1.getLength(),xfs2.getLength());
			filesystem_t::close(xfs1);
			filesystem_t::close(xfs2);
		}

		UNITTEST_TEST(construct2)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_Read,FileOp_Sync);
			stream_t xfs2(xfs1);
			CHECK_EQUAL(xfs1.canRead(),xfs2.canRead());
			CHECK_EQUAL(xfs1.getLength(),xfs2.getLength());
			filesystem_t::close(xfs1);
			filesystem_t::close(xfs2);
		}

		UNITTEST_TEST(construct3)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.getLength();
			stream_t xfs2 = filesystem_t::open(xfp1,FileMode_Create,FileAccess_ReadWrite,FileOp_Sync);
			u64 len2= xfs2.getLength();
			CHECK_EQUAL(xfs2.getLength(),0);
			xfs2.setLength(len1);
			CHECK_EQUAL(len1,xfs2.getLength());
		}

		UNITTEST_TEST(canRead)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.canRead(),true);
		}

		UNITTEST_TEST(canSeek)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.canSeek(),true);
		}

		UNITTEST_TEST(canWrite)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.canWrite(),true);
			stream_t xfs2 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_Read,FileOp_Sync);
			CHECK_EQUAL(xfs2.canWrite(),false);
		}

		UNITTEST_TEST(isOpen)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.isOpen(),true);

			const char* str2 = "TEST:\\test\\test\\test.txt";
			filepath_t xfp2 = filesystem_t::filepath(str2);
			stream_t xfs2 = filesystem_t::open(xfp2,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs2.isOpen(),false); 
		}

		UNITTEST_TEST(isAsync)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.isAsync(),false);

			stream_t xfs2 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Async);
			CHECK_EQUAL(xfs2.isAsync(),true);

		}

		UNITTEST_TEST(getLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.getLength();
			stream_t xfs2 = filesystem_t::open(xfp1,FileMode_Create,FileAccess_ReadWrite,FileOp_Sync);
			u64 len2= xfs2.getLength();
			CHECK_EQUAL(xfs2.getLength(),0);
			xfs2.setLength(10);
			CHECK_EQUAL(10,xfs2.getLength());
			xfs2.setLength(len1);
		}

		UNITTEST_TEST(setLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.getLength();
			stream_t xfs2 = filesystem_t::open(xfp1,FileMode_Create,FileAccess_ReadWrite,FileOp_Sync);
			u64 len2= xfs2.getLength();
			CHECK_EQUAL(xfs2.getLength(),0);
			xfs2.setLength(20);
			CHECK_EQUAL(20,xfs2.getLength());
			xfs2.setLength(len1);
		}

		UNITTEST_TEST(getPos)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 pos1 = xfs1.getPos();
			CHECK_EQUAL(pos1,0);
		}

		UNITTEST_TEST(setPos)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xfs1.setPos(10);
			CHECK_EQUAL(xfs1.getPos(),10);
			xfs1.setPos(0);
			CHECK_EQUAL(xfs1.getPos(),0);
		}

		UNITTEST_TEST(close)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xfs1.close();
			CHECK_EQUAL(xfs1.isOpen(),false);
		}

		UNITTEST_TEST(read)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.getPos(),0);
			xbyte buffer1[8124];
			u64 fileLen1 = xfs1.read(buffer1, 10); 
			CHECK_EQUAL(10,fileLen1);
		}

		UNITTEST_TEST(readByte)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte byte;
			u64 len = xfs1.read(&byte, 1);
			CHECK_EQUAL(byte,'T');
			CHECK_EQUAL(len,1);
		}

		UNITTEST_TEST(write)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			CHECK_EQUAL(xfs1.getPos(),0);
			xbyte buffer1[10];
			u64 fileLen1 = xfs1.read(buffer1, 10);
			CHECK_EQUAL(fileLen1,10);
			xbyte buffer_write[10] = "abcdefghi";
			xfs1.write(buffer_write, 10);

			xbyte buffer_read[10];
			u64 fileLen3 = xfs1.read(buffer_read, 10);
			CHECK_EQUAL(fileLen3,10);
			for(int n = 0; n<10;++n)
			{
				CHECK_EQUAL(buffer_write[n],buffer_read[n]);
			}
			xfs1.write(buffer1, 10);
		}

		UNITTEST_TEST(writeByte)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			u64 len1 = xfs1.write((xbyte const*)"M", 1);
			xbyte byte;
			u64 len2 = xfs1.read(&byte, 1);
			CHECK_EQUAL(byte,'M');
			CHECK_EQUAL(len2,len1);
			xfs1.write((xbyte const*)"T", 1);
		}

		UNITTEST_TEST(copyTo1)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer1[10];
			xfs1.read(buffer1, 10);
			const char* str2 = "Test:\\copyTo1\\copy.txt";
			filepath_t xfp2 = filesystem_t::filepath(str2);
			stream_t fpTemp;
			CHECK_FALSE(fileinfo_t::sExists(xfp2));
			CHECK_TRUE(fileinfo_t::sCreate(xfp2,fpTemp));
			CHECK_TRUE(fileinfo_t::sExists(xfp2));
			stream_t xfs2 = filesystem_t::open(xfp2,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte stream_buffer_data[1024];
			buffer_t stream_buffer(1024, stream_buffer_data);
			xstream_copy(xfs1, xfs2, stream_buffer);
			xbyte buffer2[10];
			xfs2.read(buffer2, 10);
			for(int n =0; n < 10; ++n)
			{
				CHECK_EQUAL(buffer1[n],buffer2[n]);
			}
			CHECK_TRUE(fileinfo_t::sDelete(xfp2));
			CHECK_FALSE(fileinfo_t::sExists(xfp2));
		}

		UNITTEST_TEST(copyTo2)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			filepath_t xfp1 = filesystem_t::filepath(str1);
			stream_t xfs1 = filesystem_t::open(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer1[10];
			xfs1.read(buffer1, 10);
			const char* str2 = "Test:\\copyTo1\\copy.txt";
			filepath_t xfp2 = filesystem_t::filepath(str2);
			stream_t fpTemp;
			CHECK_FALSE(fileinfo_t::sExists(xfp2));
			CHECK_TRUE(fileinfo_t::sCreate(xfp2,fpTemp));
			CHECK_TRUE(fileinfo_t::sExists(xfp2));
			stream_t xfs2 = filesystem_t::open(xfp2,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);

			xbyte stream_buffer_data[1024];
			buffer_t stream_buffer(1024, stream_buffer_data);
			xstream_copy(xfs1, xfs2, stream_buffer);

			xbyte buffer2[20];
			u64 readLen = xfs2.read(buffer2, 20);
			CHECK_EQUAL(readLen,10);
			for(int n = 0; n < 10; ++n)
			{
				CHECK_EQUAL(buffer1[n],buffer2[n]);
			}
			CHECK_TRUE(fileinfo_t::sDelete(xfp2));
			CHECK_FALSE(fileinfo_t::sExists(xfp2));
		}
	}
}
UNITTEST_SUITE_END
