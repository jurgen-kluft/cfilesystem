#include "xbase\x_types.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\x_threading.h"

#include "xfilesystem\x_async_result.h"
#include "xfilesystem\private\x_devicealias.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_fileasync.h"
#include "xfilesystem\x_filestream.h"

using namespace xcore;
using namespace xcore::xfilesystem;

	
UNITTEST_SUITE_BEGIN(filesystem_common)
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
		UNITTEST_FIXTURE_TEARDOWN() 
		{

		}

		// main 


		


		// TODO: implement async tests properly
		
		/*
		UNITTEST_TEST(beginRead)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer1[100];
			xfs1.beginRead(buffer1,0,100, callbackRead_TEST);

		}

		UNITTEST_TEST(beginWrite)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer[10];
			xfs1.read(buffer,0,10);

			xbyte buffer1[10] = "abcdefgh";
			//			memset(buffer1,0,sizeof(buffer1));
			xfs1.beginWrite(buffer1,0,10, callbackWrite_TEST);
			xbyte buffer2[10];
			xfs1.read(buffer2,0,10);
			for (int n = 0; n<10;++n)
			{
				CHECK_EQUAL(buffer1[n],buffer2[n]);
			}
			xfs1.write(buffer,0,10);
		}
		*/

		//============================================


		UNITTEST_TEST(createSystemPath)
		{
			char systemFilenameBuffer1[512];
			xcstring systemFilename1(systemFilenameBuffer1, sizeof(systemFilenameBuffer1));
			xfilesystem::xfs_common::s_instance()->createSystemPath("sdfsfsa:\\app.config", systemFilename1);

			char systemFilenameBuffer2[512];
			xcstring systemFilename2(systemFilenameBuffer2, sizeof(systemFilenameBuffer2));
			xfilesystem::xfs_common::s_instance()->createSystemPath("curdir:\\app.config", systemFilename2);

			CHECK_EQUAL(0, x_strCompare(systemFilename1.c_str(), systemFilename2.c_str()));
		}

		UNITTEST_TEST(exists)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			CHECK_FALSE(xfilesystem::xfs_common::s_instance()->exists(str));

			const char* str1 = "INVALID:\\xfilesystem_test\the_folder\\tech.txt";
			CHECK_FALSE(xfilesystem::xfs_common::s_instance()->exists(str1));

			const char* str2 = "TEST:\\textfiles\\docs\\tech.txt";
			CHECK_TRUE(xfilesystem::xfs_common::s_instance()->exists(str2));
		}

		UNITTEST_TEST(getLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfilesystem::xfs_common::s_instance()->open(str1,true,false);
			xbyte buffer1[8192];
			u64 fileLen1 = xfilesystem::xfs_common::s_instance()->read(uHandle1,u64(0),sizeof(buffer1),buffer1,NULL);
			CHECK_EQUAL(xfilesystem::xfs_common::s_instance()->getLength(uHandle1),fileLen1);
			xfilesystem::xfs_common::s_instance()->close(uHandle1,NULL);

			const char* str2 = "TEST:\\textfiles\\authors.txt";
			s32 uHandle2 = xfilesystem::xfs_common::s_instance()->open(str2,true,false);
			xbyte buffer2[8192];
			u64 fileLen2 = xfilesystem::xfs_common::s_instance()->read(uHandle2,u64(0),sizeof(buffer2),buffer2,NULL);
			CHECK_EQUAL(xfilesystem::xfs_common::s_instance()->getLength(uHandle2),fileLen2);
			xfilesystem::xfs_common::s_instance()->close(uHandle2,NULL);
		}

		UNITTEST_TEST(setLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfilesystem::xfs_common::s_instance()->open(str1,true,false);
			xbyte buffer1[8192];
			u64 fileLen1 = xfilesystem::xfs_common::s_instance()->read(uHandle1,u64(0),sizeof(buffer1),buffer1,NULL);
			xfilesystem::xfs_common::s_instance()->setLength(uHandle1,fileLen1+1);
			CHECK_EQUAL(xfilesystem::xfs_common::s_instance()->getLength(uHandle1),fileLen1+1);
			xfilesystem::xfs_common::s_instance()->close(uHandle1,NULL);
		}

		UNITTEST_TEST(caps)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath path(str1);
			const xdevicealias* alias = xdevicealias::sFind(path);
			const xfiledevice* file_device = alias->device();
			bool can_read;
			bool can_write;
			bool can_seek;
			bool can_async;
			bool result = xfilesystem::xfs_common::s_instance()->caps(path,can_read,can_write,can_seek,can_async);
			CHECK_EQUAL(can_read,true);
			CHECK_EQUAL(can_write,file_device->canWrite());
			CHECK_EQUAL(can_seek,file_device->canSeek());
			CHECK_EQUAL(can_async,true);
		}

		UNITTEST_TEST(hasLastError)
		{
			xfs_common::s_instance()->setLastError(FILE_ERROR_NO_FILE);
			CHECK_TRUE(xfs_common::s_instance()->hasLastError());
		}

		UNITTEST_TEST(clearLastError)
		{
			xfs_common::s_instance()->setLastError(FILE_ERROR_NO_FILE);
			CHECK_TRUE(xfs_common::s_instance()->hasLastError());
			xfs_common::s_instance()->clearLastError();
			CHECK_FALSE(xfs_common::s_instance()->hasLastError());
		}

		UNITTEST_TEST(getLastError)
		{
			xfs_common::s_instance()->setLastError(FILE_ERROR_NO_FILE);
			CHECK_EQUAL(xfs_common::s_instance()->getLastError(),FILE_ERROR_NO_FILE);

			xfs_common::s_instance()->setLastError(FILE_ERROR_BADF);
			CHECK_EQUAL(xfs_common::s_instance()->getLastError(),FILE_ERROR_BADF);
		}

		UNITTEST_TEST(getLastErrorStr)
		{
			xfs_common::s_instance()->setLastError(FILE_ERROR_OK);
			xfs_common::s_instance()->setLastError(FILE_ERROR_OK);
			CHECK_EQUAL(xfs_common::s_instance()->getLastErrorStr(),xfs_common::s_instance()->getLastErrorStr());
			CHECK_FALSE(xfs_common::s_instance()->hasLastError());

			xfs_common::s_instance()->setLastError(FILE_ERROR_NO_FILE);
			xfs_common::s_instance()->setLastError(FILE_ERROR_BADF);
			CHECK_NOT_EQUAL(xfs_common::s_instance()->getLastErrorStr(),xfs_common::s_instance()->getLastErrorStr());
			CHECK_FALSE(xfs_common::s_instance()->hasLastError());
		}

		UNITTEST_TEST(heapAlloc)
		{
			char* filename = (char*)heapAlloc(s32(32),X_ALIGNMENT_DEFAULT);
			const char* temp = "sssssssssssssssssssssssssssssss";
			x_memset(filename,'s',31);
			filename[31] = '\0';
			CHECK_EQUAL(filename,temp);
			heapFree(filename);
			filename = NULL;

			char* file = (char*)heapAlloc(3,X_ALIGNMENT_DEFAULT);
			const char* t = "ab";
			file[0] = 'a';
			file[1] = 'b';
			file[2] = '\0';
			CHECK_EQUAL(file,t);
			heapFree(file);
		}

		UNITTEST_TEST(heapFree)
		{
			char* file = (char*)heapAlloc(4,X_ALIGNMENT_DEFAULT);
			const char* t = "abc";
			file[0] = 'a';
			file[1] = 'b';
			file[2] = 'c';
			file[3] = '\0';
			CHECK_EQUAL(file,t);
			heapFree(file);
		}

		UNITTEST_TEST(getFileInfo)
		{
			for (u32 uFile = 0;uFile < 4; ++ uFile )
			{
				if (xfs_common::s_instance()->getFileInfo(uFile))
				{
					CHECK_TRUE(xfs_common::s_instance()->getFileInfo(uFile));
				}
			}
		}

		UNITTEST_TEST(popFreeFileSlot)
		{
			s32 uHandle1 = xfs_common::s_instance()->popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle1);
			s32 uHandle2 = xfs_common::s_instance()->popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle2);
			CHECK_TRUE(xfs_common::s_instance()->pushFreeFileSlot(uHandle1));
			CHECK_TRUE(xfs_common::s_instance()->pushFreeFileSlot(uHandle2));
		}

		UNITTEST_TEST(pushFreeFileSlot)
		{
			s32 uHandle1 = xfs_common::s_instance()->popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle1);
			s32 uHandle2 = xfs_common::s_instance()->popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle2);
			CHECK_TRUE(xfs_common::s_instance()->pushFreeFileSlot(uHandle1));
			CHECK_TRUE(xfs_common::s_instance()->pushFreeFileSlot(uHandle2));
		}

		UNITTEST_TEST(getAsyncIOData)
		{
			CHECK_EQUAL(xfs_common::s_instance()->getAsyncIOData(0)->getFileIndex() , -1);
			CHECK_EQUAL(xfs_common::s_instance()->getAsyncIOData(1)->getFileIndex() , -1);
			CHECK_EQUAL(xfs_common::s_instance()->getAsyncIOData(2)->getFileIndex() , -1);
			CHECK_EQUAL(xfs_common::s_instance()->getAsyncIOData(3)->getFileIndex() , -1);
			CHECK_EQUAL(xfs_common::s_instance()->getAsyncIOData(4)->getFileIndex() , -1); // NOTE: before this was CHECK_NOT_EQUAL. not sure why
		}

		UNITTEST_TEST(popFreeAsyncIO)
		{
			xfileasync* xfileasync1 = xfs_common::s_instance()->popFreeAsyncIO(true);
			xfileasync* xfileasync2 = xfs_common::s_instance()->popFreeAsyncIO(true);
			xfileasync* xfileasync3 = xfs_common::s_instance()->popFreeAsyncIO(true);
			xfileasync* xfileasync4 = xfs_common::s_instance()->popFreeAsyncIO(true);
			CHECK_EQUAL(xfileasync1->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync2->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync3->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync4->getFileIndex(),-1);
			// if continue use popFreeAsyncIO(false) , it will crash
			// else continue use popFreeAsyncIO(true),it will waite till freeAsyncIOList not empty
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync1);
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync2);
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync3);
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync4);
		}

		UNITTEST_TEST(pushFreeAsyncIO)
		{
			xfileasync* xfileasync1 = xfs_common::s_instance()->popFreeAsyncIO(false);
			xfileasync* xfileasync2 = xfs_common::s_instance()->popFreeAsyncIO(false);
			xfileasync* xfileasync3 = xfs_common::s_instance()->popFreeAsyncIO(false);
			xfileasync* xfileasync4 = xfs_common::s_instance()->popFreeAsyncIO(false);
			CHECK_EQUAL(xfileasync1->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync2->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync3->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync4->getFileIndex(),-1);
			// if continue use popFreeAsyncIO(false) , it will crash
			// else continue use popFreeAsyncIO(true),it will waite till freeAsyncIOList not empty
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync1);
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync2);
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync3);
			xfs_common::s_instance()->pushFreeAsyncIO(xfileasync4);
		}

		UNITTEST_TEST(popAsyncIO)
		{
			xfileasync* xfileasync1 = new xfileasync();
			xfileasync1->clear();
			xfs_common::s_instance()->pushAsyncIO(xfileasync1);
			xfileasync* xfileasync2 = xfs_common::s_instance()->popAsyncIO();
			CHECK_EQUAL(xfileasync1->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync1,xfileasync2);
			delete xfileasync1;
		}

		UNITTEST_TEST(pushAsyncIO)
		{
			xfileasync* xfileasync1 = new xfileasync();
			xfileasync1->clear();
			xfs_common::s_instance()->pushAsyncIO(xfileasync1);
			xfileasync* xfileasync2 = xfs_common::s_instance()->popAsyncIO();
			CHECK_EQUAL(xfileasync1->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync1,xfileasync2);
			delete xfileasync1;
		}

		UNITTEST_TEST(testAsyncId)
		{
			xfileasync* xfileasync1 = new xfileasync();
			xfileasync* xfileasync2 = new xfileasync();
			xfileasync1->clear();
			xfileasync2->clear();
			xasync_id id1 = xfs_common::s_instance()->pushAsyncIO(xfileasync1);
			xasync_id id2 = xfs_common::s_instance()->pushAsyncIO(xfileasync2);
			CHECK_EQUAL(xfs_common::s_instance()->testAsyncId(id1),0);
			CHECK_EQUAL(xfs_common::s_instance()->testAsyncId(id2),0);
			xfs_common::s_instance()->popAsyncIO();
			xfs_common::s_instance()->popAsyncIO();
			CHECK_EQUAL(xfs_common::s_instance()->testAsyncId(id1),-1);
			CHECK_EQUAL(xfs_common::s_instance()->testAsyncId(id2),-1);
			delete xfileasync1;
			delete xfileasync2;
		}


		UNITTEST_TEST(isPathUNIXStyle)
		{
#if defined(TARGET_PC) || defined(TARGET_360)
			CHECK_FALSE(xfs_common::s_instance()->isPathUNIXStyle()); 
#endif
#if defined(TARGET_PS3) || defined(TARGET_PSP) || defined(TARGET_WII) || defined(TARGET_3DS)
			CHECK_TRUE(xfs_common::s_instance()->isPathUNIXStyle());
#endif
		}

		UNITTEST_TEST(setLastError)
		{
			xfs_common::s_instance()->setLastError(FILE_ERROR_MAX_FILES);
			CHECK_EQUAL(xfs_common::s_instance()->getLastError(),FILE_ERROR_MAX_FILES);

			xfs_common::s_instance()->setLastError(FILE_ERROR_DEVICE);
			CHECK_EQUAL(xfs_common::s_instance()->getLastError(),FILE_ERROR_DEVICE);
		}

		UNITTEST_TEST(open)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 =  xfs_common::s_instance()->open(str1,true,false);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle1);
			xfs_common::s_instance()->close(uHandle1,NULL);	

			const char* str2 = "TEST:\\textfiles\\authors.txt";
			s32 uHandle2 = xfs_common::s_instance()->open(str2,true,true);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle2);
			xfs_common::s_instance()->close(uHandle2,NULL);	
		}

		UNITTEST_TEST(read)
		{
			const char* str1 = "TEST:\\textfiles\\authors.txt";
			s32 uHandle1 = xfs_common::s_instance()->open(str1,true,false);
			xbyte buffer1[8192];
			u64 fileLen1 = xfilesystem::xfs_common::s_instance()->read(uHandle1,u64(0),sizeof(buffer1),buffer1,NULL);
			CHECK_TRUE(fileLen1);
			xfs_common::s_instance()->close(uHandle1,NULL);

			// TODO rework this later
			/*
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_2 = &event_factory_temp;

			setEventFactory(event_factory_2);
			xiasync_result_test xiasync_result_test_1;
			xiasync_result* xiasync_result_temp1 = &xiasync_result_test_1;
			s32 uHandle2 = open(str1,true,false);
			u64 fileLen2 = xfilesystem::read(uHandle2,u64(0),sizeof(buffer1),buffer1,&xiasync_result_temp1);
			CHECK_FALSE(fileLen2);
			xiasync_result_test xiasync_result_test_2;
			xiasync_result* xiasync_result_temp2 = &xiasync_result_test_2;
			close(uHandle2,&xiasync_result_temp2);
			//close(uHandle2,NULL);
			*/
		}

		UNITTEST_TEST(write)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfs_common::s_instance()->open(str1,true,true);
			xbyte buffer1[] = { "a unittest writing data to a file" };
			xbyte bufferRead[8192];
			u64 fileLen1 = xfilesystem::xfs_common::s_instance()->read(uHandle1,u64(0),sizeof(bufferRead),bufferRead,NULL);
			u64 writeLen1 = xfilesystem::xfs_common::s_instance()->write(uHandle1,fileLen1,sizeof(buffer1),buffer1,NULL);
			CHECK_EQUAL(writeLen1 , sizeof(buffer1));

			xbyte buffer2[8192];
			u64 fileLen2 = xfilesystem::xfs_common::s_instance()->read(uHandle1,u64(0),sizeof(buffer2),buffer2,NULL);
			CHECK_NOT_EQUAL(fileLen1,fileLen2);
			CHECK_EQUAL(fileLen2,fileLen1+sizeof(buffer1));
			xfs_common::s_instance()->close(uHandle1,NULL);	

			/*
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_2 = &event_factory_temp;

			setEventFactory(event_factory_2);
			xiasync_result_test xiasync_result_test_1;
			xiasync_result* xiasync_result_temp1 = &xiasync_result_test_1;
			s32 uHandle2 = open(str1,true,true);
			u64 writeLen2 = xfilesystem::write(uHandle2,fileLen2,sizeof(buffer1),buffer1,&xiasync_result_temp1);
			CHECK_EQUAL(writeLen2,0);
			close(uHandle2,NULL);
			*/
		}

		UNITTEST_TEST(getpos)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfs_common::s_instance()->open(str1,true,false);
			u64 pos = xfs_common::s_instance()->getpos(uHandle1);
			CHECK_EQUAL(pos,u64(0));
			xfs_common::s_instance()->close(uHandle1,NULL);	
		}

		UNITTEST_TEST(setpos)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfs_common::s_instance()->open(str1,true,false);
			xfs_common::s_instance()->setpos(uHandle1,u64(20));
			u64 pos = xfs_common::s_instance()->getpos(uHandle1);
			CHECK_EQUAL(pos,u64(20));

			xfs_common::s_instance()->setpos(uHandle1,u64(0));
			u64 pos1 = xfs_common::s_instance()->getpos(uHandle1);
			CHECK_EQUAL(pos1,u64(0));
			xfs_common::s_instance()->close(uHandle1,NULL);	
		}

		UNITTEST_TEST(close)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfs_common::s_instance()->open(str1,true,false);
			xfs_common::s_instance()->close(uHandle1,NULL);
			CHECK_EQUAL(uHandle1,INVALID_FILE_HANDLE);

			s32 uHandle2 = xfs_common::s_instance()->open(str1,true,false);
			xfs_common::s_instance()->close(uHandle2,NULL);
		}

		UNITTEST_TEST(save)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfs_common::s_instance()->open(str1,true,false);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE,uHandle1);
			xfs_common::s_instance()->close(uHandle1,NULL);
			xbyte buffer1[] = { "a unittest saving data to a file" };
			xfs_common::s_instance()->save(str1,buffer1,sizeof(buffer1));
			s32 uHandle2 = xfs_common::s_instance()->open(str1,true,true);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE,uHandle2);
			xfs_common::s_instance()->close(uHandle2,NULL);
		}

		UNITTEST_TEST(closeAndDelete)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			s32 uHandle1 = xfs_common::s_instance()->open(str1,true,false);
			xfs_common::s_instance()->closeAndDelete(uHandle1,NULL);
			CHECK_EQUAL(uHandle1,INVALID_FILE_HANDLE);
		}
	}
}
UNITTEST_SUITE_END

