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
#include "xfilesystem\x_iasync_result.h"
#include "xfilesystem\x_filestream.h"

using namespace xcore;
namespace xcore
{
	using namespace xfilesystem;
	namespace xfilesystem
	{
		class xevent_test : public xevent
		{
		public:
			virtual				~xevent_test()						{ }

			virtual void		set()										{ }
			virtual void		wait()									{ }
			virtual void		signal()								{ }
		};

		static xevent_test event_test ;

		class xevent_factory_test : public xevent_factory
		{
		public:
			virtual				~xevent_factory_test()				{ }

			virtual xevent*		construct()						{ xevent* event = new xevent_test; return event;}		
			virtual void		destruct(xevent* event)				{ delete event; event = NULL; }
		};

		class xiasync_result_test : public xiasync_result
		{
		public:
			xiasync_result_test();
			virtual			~xiasync_result_test()					{ }

			void			init(xasync_id nAsyncId, u32 nFileHandle, xevent* pEvent);

			virtual bool	checkForCompletion();
			virtual void	waitForCompletion();

			virtual u64		getResult() const;

			virtual void	clear();
			virtual s32		hold();
			virtual s32		release();
			virtual void	destroy();

			XFILESYSTEM_OBJECT_NEW_DELETE()
		private:
			s32				mRefCount;		/// This needs to be an atomic integer
			xasync_id		mAsyncId;
			u32				mFileHandle;
			u64				mResult;
			xevent*			mEvent;
		};

		//================

		xiasync_result_test::xiasync_result_test()
			: mRefCount(0)
			, mAsyncId(0)
			, mFileHandle(INVALID_FILE_HANDLE)
			, mResult(0)
			, mEvent(NULL)
		{
		}

		void			xiasync_result_test::init(xasync_id nAsyncId, u32 nFileHandle, xevent* pEvent)
		{
			mRefCount = 0;
			mAsyncId = nAsyncId;
			mFileHandle = nFileHandle;
			mResult = 0;
			mEvent = pEvent;
		}

		bool			xiasync_result_test::checkForCompletion()
		{
			return mFileHandle == INVALID_FILE_HANDLE || testAsyncId(mAsyncId) == -1;
		}

		void			xiasync_result_test::waitForCompletion()
		{
			if (mFileHandle != INVALID_FILE_HANDLE)
			{
				xfiledata* fileInfo = getFileInfo(mFileHandle);
				if (checkForCompletion() == false)
				{
					mEvent->wait();
				}
			}
		}

		u64				xiasync_result_test::getResult() const
		{
			return mResult;
		}

		void			xiasync_result_test::clear()
		{
			mFileHandle = INVALID_FILE_HANDLE;
		}

		s32				xiasync_result_test::hold()
		{
			return ++mRefCount;
		}

		s32				xiasync_result_test::release()
		{
			return --mRefCount;
		}

		void			xiasync_result_test::destroy()
		{
			// Push us back in the free list
			pushAsyncResult(this);
		}
		//================
	}
}

	
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
		UNITTEST_FIXTURE_TEARDOWN() {}

		// main 


		


		UNITTEST_TEST(beginRead)
		{
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_1 = &event_factory_temp;
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			xfilepath xfp1(str1);
			xfilestream xfs1(xfp1,FileMode_Open,FileAccess_ReadWrite,FileOp_Sync);
			xbyte buffer1[100];
			xfs1.beginRead(buffer1,0,100, callbackRead_TEST);

		}

		UNITTEST_TEST(beginWrite)
		{
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_1 = &event_factory_temp;
			setEventFactory(event_factory_1);
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

		//============================================


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

		UNITTEST_TEST(exists)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			CHECK_FALSE(xfilesystem::exists(str));

			const char* str1 = "INVALID:\\xfilesystem_test\the_folder\\tech.txt";
			CHECK_FALSE(xfilesystem::exists(str1));

			const char* str2 = "TEST:\\textfiles\\docs\\tech.txt";
			CHECK_TRUE(xfilesystem::exists(str2));
		}

		UNITTEST_TEST(getLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 = xfilesystem::open(str1,true,false);
			xbyte buffer1[8192];
			u64 fileLen1 = xfilesystem::read(uHandle1,u64(0),sizeof(buffer1),buffer1,NULL);
			CHECK_EQUAL(xfilesystem::getLength(uHandle1),fileLen1);
			xfilesystem::close(uHandle1,NULL);

			const char* str2 = "TEST:\\textfiles\\authors.txt";
			u32 uHandle2 = xfilesystem::open(str2,true,false);
			xbyte buffer2[8192];
			u64 fileLen2 = xfilesystem::read(uHandle2,u64(0),sizeof(buffer2),buffer2,NULL);
			CHECK_EQUAL(xfilesystem::getLength(uHandle2),fileLen2);
			xfilesystem::close(uHandle2,NULL);
		}

		UNITTEST_TEST(setLength)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 = xfilesystem::open(str1,true,false);
			xbyte buffer1[8192];
			u64 fileLen1 = xfilesystem::read(uHandle1,u64(0),sizeof(buffer1),buffer1,NULL);
			xfilesystem::setLength(uHandle1,fileLen1+1);
			CHECK_EQUAL(xfilesystem::getLength(uHandle1),fileLen1+1);
			xfilesystem::close(uHandle1,NULL);
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
			bool result = xfilesystem::caps(path,can_read,can_write,can_seek,can_async);
			CHECK_EQUAL(can_read,true);
			CHECK_EQUAL(can_write,file_device->canWrite());
			CHECK_EQUAL(can_seek,file_device->canSeek());
			CHECK_EQUAL(can_async,true);
		}

		UNITTEST_TEST(hasLastError)
		{
			setLastError(FILE_ERROR_NO_FILE);
			CHECK_TRUE(hasLastError());
		}

		UNITTEST_TEST(clearLastError)
		{
			setLastError(FILE_ERROR_NO_FILE);
			CHECK_TRUE(hasLastError());
			clearLastError();
			CHECK_FALSE(hasLastError());
		}

		UNITTEST_TEST(getLastError)
		{
			setLastError(FILE_ERROR_NO_FILE);
			CHECK_EQUAL(getLastError(),FILE_ERROR_NO_FILE);

			setLastError(FILE_ERROR_BADF);
			CHECK_EQUAL(getLastError(),FILE_ERROR_BADF);
		}

		UNITTEST_TEST(getLastErrorStr)
		{
			setLastError(FILE_ERROR_OK);
			setLastError(FILE_ERROR_OK);
			CHECK_EQUAL(getLastErrorStr(),getLastErrorStr());
			CHECK_FALSE(hasLastError());

			setLastError(FILE_ERROR_NO_FILE);
			setLastError(FILE_ERROR_BADF);
			CHECK_NOT_EQUAL(getLastErrorStr(),getLastErrorStr());
			CHECK_FALSE(hasLastError());
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
				if (getFileInfo(uFile))
				{
					CHECK_TRUE(getFileInfo(uFile));
				}
			}
		}

		UNITTEST_TEST(popFreeFileSlot)
		{
			u32 uHandle1 = popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle1);
			u32 uHandle2 = popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle2);
			CHECK_TRUE(pushFreeFileSlot(uHandle1));
			CHECK_TRUE(pushFreeFileSlot(uHandle2));
		}

		UNITTEST_TEST(pushFreeFileSlot)
		{
			u32 uHandle1 = popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle1);
			u32 uHandle2 = popFreeFileSlot();
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle2);
			CHECK_TRUE(pushFreeFileSlot(uHandle1));
			CHECK_TRUE(pushFreeFileSlot(uHandle2));
		}

		UNITTEST_TEST(getAsyncIOData)
		{
			CHECK_EQUAL(getAsyncIOData(0)->getFileIndex() , -1);
			CHECK_EQUAL(getAsyncIOData(1)->getFileIndex() , -1);
			CHECK_EQUAL(getAsyncIOData(2)->getFileIndex() , -1);
			CHECK_EQUAL(getAsyncIOData(3)->getFileIndex() , -1);
			CHECK_NOT_EQUAL(getAsyncIOData(4)->getFileIndex() , -1);
		}

		UNITTEST_TEST(popFreeAsyncIO)
		{
			xfileasync* xfileasync1 = popFreeAsyncIO(true);
			xfileasync* xfileasync2 = popFreeAsyncIO(true);
			xfileasync* xfileasync3 = popFreeAsyncIO(true);
			xfileasync* xfileasync4 = popFreeAsyncIO(true);
			CHECK_EQUAL(xfileasync1->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync2->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync3->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync4->getFileIndex(),-1);
			// if continue use popFreeAsyncIO(false) , it will crash
			// else continue use popFreeAsyncIO(true),it will waite till freeAsyncIOList not empty
			pushFreeAsyncIO(xfileasync1);
			pushFreeAsyncIO(xfileasync2);
			pushFreeAsyncIO(xfileasync3);
			pushFreeAsyncIO(xfileasync4);
		}

		UNITTEST_TEST(pushFreeAsyncIO)
		{
			xfileasync* xfileasync1 = popFreeAsyncIO(false);
			xfileasync* xfileasync2 = popFreeAsyncIO(false);
			xfileasync* xfileasync3 = popFreeAsyncIO(false);
			xfileasync* xfileasync4 = popFreeAsyncIO(false);
			CHECK_EQUAL(xfileasync1->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync2->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync3->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync4->getFileIndex(),-1);
			// if continue use popFreeAsyncIO(false) , it will crash
			// else continue use popFreeAsyncIO(true),it will waite till freeAsyncIOList not empty
			pushFreeAsyncIO(xfileasync1);
			pushFreeAsyncIO(xfileasync2);
			pushFreeAsyncIO(xfileasync3);
			pushFreeAsyncIO(xfileasync4);
		}

		UNITTEST_TEST(popAsyncIO)
		{
			xfileasync* xfileasync1 = new xfileasync();
			xfileasync1->clear();
			pushAsyncIO(xfileasync1);
			xfileasync* xfileasync2 = popAsyncIO();
			CHECK_EQUAL(xfileasync1->getFileIndex(),-1);
			CHECK_EQUAL(xfileasync1,xfileasync2);
			delete xfileasync1;
		}

		UNITTEST_TEST(pushAsyncIO)
		{
			xfileasync* xfileasync1 = new xfileasync();
			xfileasync1->clear();
			pushAsyncIO(xfileasync1);
			xfileasync* xfileasync2 = popAsyncIO();
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
			xasync_id id1 = pushAsyncIO(xfileasync1);
			xasync_id id2 = pushAsyncIO(xfileasync2);
			CHECK_EQUAL(testAsyncId(id1),0);
			CHECK_EQUAL(testAsyncId(id2),0);
			popAsyncIO();
			popAsyncIO();
			CHECK_EQUAL(testAsyncId(id1),-1);
			CHECK_EQUAL(testAsyncId(id2),-1);
			delete xfileasync1;
			delete xfileasync2;
		}

		UNITTEST_TEST(popAsyncResult)
		{
			xiasync_result* result_1 = new xiasync_result_test();
			xiasync_result* result_2 = new xiasync_result_test();

			xiasync_result* result1 = popAsyncResult();
			xiasync_result* result2 = popAsyncResult();
			xiasync_result* result3 = popAsyncResult();
			xiasync_result* result4 = popAsyncResult();

			CHECK_EQUAL(result1->getResult(),0);
			CHECK_EQUAL(result2->getResult(),0);
			CHECK_EQUAL(result3->getResult(),0);
			CHECK_EQUAL(result4->getResult(),0);

			pushAsyncResult(result_1);
			pushAsyncResult(result_2);
			xiasync_result* result_check_1 = popAsyncResult();
			xiasync_result* result_check_2 = popAsyncResult();
			CHECK_EQUAL(result_1,result_check_1);
			CHECK_EQUAL(result_2,result_check_2);

			pushAsyncResult(result1);
			pushAsyncResult(result2);
			pushAsyncResult(result3);
			pushAsyncResult(result4);

			delete result_1;
			delete result_2;
		}

		UNITTEST_TEST(pushAsyncResult)
		{
			xiasync_result* result_1 = new xiasync_result_test();
			xiasync_result* result_2 = new xiasync_result_test();

			xiasync_result* result1 = popAsyncResult();
			xiasync_result* result2 = popAsyncResult();
			xiasync_result* result3 = popAsyncResult();
			xiasync_result* result4 = popAsyncResult();

			CHECK_EQUAL(result1->getResult(),0);
			CHECK_EQUAL(result2->getResult(),0);
			CHECK_EQUAL(result3->getResult(),0);
			CHECK_EQUAL(result4->getResult(),0);

			pushAsyncResult(result_1);
			pushAsyncResult(result_2);
			xiasync_result* result_check_1 = popAsyncResult();
			xiasync_result* result_check_2 = popAsyncResult();
			CHECK_EQUAL(result_1,result_check_1);
			CHECK_EQUAL(result_2,result_check_2);

			pushAsyncResult(result1);
			pushAsyncResult(result2);
			pushAsyncResult(result3);
			pushAsyncResult(result4);

			delete result_1;
			delete result_2;
		}

		UNITTEST_TEST(isPathUNIXStyle)
		{
#if defined(TARGET_PC) || defined(TARGET_360)
			CHECK_FALSE(isPathUNIXStyle()); 
#endif
#if defined(TARGET_PS3) || defined(TARGET_PSP) || defined(TARGET_WII) || defined(TARGET_3DS)
			CHECK_TRUE(isPathUNIXStyle());
#endif
		}

		UNITTEST_TEST(setLastError)
		{
			setLastError(FILE_ERROR_MAX_FILES);
			CHECK_EQUAL(getLastError(),FILE_ERROR_MAX_FILES);

			setLastError(FILE_ERROR_DEVICE);
			CHECK_EQUAL(getLastError(),FILE_ERROR_DEVICE);
		}

		UNITTEST_TEST(open)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 =  open(str1,true,false);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle1);
			close(uHandle1,NULL);	

			const char* str2 = "TEST:\\textfiles\\authors.txt";
			u32 uHandle2 = open(str2,true,true);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE , uHandle2);
			close(uHandle2,NULL);	

			/*
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_2 = &event_factory_temp;
			setEventFactory(event_factory_2);

			xiasync_result_test xiasync_result_test_1;
			xiasync_result* xiasync_result_temp1 = &xiasync_result_test_1;
			u32 uHandle3 = open(str2,true,true,&xiasync_result_temp1);

			xiasync_result_test xiasync_result_test_2;
			xiasync_result* xiasync_result_temp2 = &xiasync_result_test_2;
			close(uHandle3,&xiasync_result_temp2);
			//close(uHandle3,NULL);
			*/
		}

		UNITTEST_TEST(read)
		{
			const char* str1 = "TEST:\\textfiles\\authors.txt";
			u32 uHandle1 = open(str1,true,false);
			xbyte buffer1[8192];
			u64 fileLen1 = xfilesystem::read(uHandle1,u64(0),sizeof(buffer1),buffer1,NULL);
			CHECK_TRUE(fileLen1);
			close(uHandle1,NULL);

			// TODO rework this later
			/*
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_2 = &event_factory_temp;

			setEventFactory(event_factory_2);
			xiasync_result_test xiasync_result_test_1;
			xiasync_result* xiasync_result_temp1 = &xiasync_result_test_1;
			u32 uHandle2 = open(str1,true,false);
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
			u32 uHandle1 = open(str1,true,true);
			xbyte buffer1[] = { "a unittest writing data to a file" };
			xbyte bufferRead[8192];
			u64 fileLen1 = xfilesystem::read(uHandle1,u64(0),sizeof(bufferRead),bufferRead,NULL);
			u64 writeLen1 = xfilesystem::write(uHandle1,fileLen1,sizeof(buffer1),buffer1,NULL);
			CHECK_EQUAL(writeLen1 , sizeof(buffer1));

			xbyte buffer2[8192];
			u64 fileLen2 = xfilesystem::read(uHandle1,u64(0),sizeof(buffer2),buffer2,NULL);
			CHECK_NOT_EQUAL(fileLen1,fileLen2);
			CHECK_EQUAL(fileLen2,fileLen1+sizeof(buffer1));
			close(uHandle1,NULL);	

			/*
			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_2 = &event_factory_temp;

			setEventFactory(event_factory_2);
			xiasync_result_test xiasync_result_test_1;
			xiasync_result* xiasync_result_temp1 = &xiasync_result_test_1;
			u32 uHandle2 = open(str1,true,true);
			u64 writeLen2 = xfilesystem::write(uHandle2,fileLen2,sizeof(buffer1),buffer1,&xiasync_result_temp1);
			CHECK_EQUAL(writeLen2,0);
			close(uHandle2,NULL);
			*/
		}

		UNITTEST_TEST(getpos)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 = open(str1,true,false);
			u64 pos = getpos(uHandle1);
			CHECK_EQUAL(pos,u64(0));
			close(uHandle1,NULL);	
		}

		UNITTEST_TEST(setpos)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 = open(str1,true,false);
			setpos(uHandle1,u64(20));
			u64 pos = getpos(uHandle1);
			CHECK_EQUAL(pos,u64(20));

			setpos(uHandle1,u64(0));
			u64 pos1 = getpos(uHandle1);
			CHECK_EQUAL(pos1,u64(0));
			close(uHandle1,NULL);	
		}

		UNITTEST_TEST(close)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 = open(str1,true,false);
			close(uHandle1,NULL);
			CHECK_EQUAL(uHandle1,INVALID_FILE_HANDLE);

			xevent_factory_test event_factory_temp;
			xevent_factory* event_factory_2 = &event_factory_temp;
			setEventFactory(event_factory_2);

			xiasync_result_test xiasync_result_test_1;
			xiasync_result* xiasync_result_temp1 = &xiasync_result_test_1;
			u32 uHandle2 = open(str1,true,false);
			close(uHandle2,NULL);
		}

		UNITTEST_TEST(save)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 = open(str1,true,false);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle1);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE,uHandle1);
			close(uHandle1,NULL);
			xbyte buffer1[] = { "a unittest saving data to a file" };
			save(str1,buffer1,sizeof(buffer1));
			u32 uHandle2 = open(str1,true,true);
			CHECK_NOT_EQUAL(PENDING_FILE_HANDLE , uHandle2);
			CHECK_NOT_EQUAL(INVALID_FILE_HANDLE,uHandle2);
			close(uHandle2,NULL);
		}

		UNITTEST_TEST(closeAndDelete)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\tech.txt";
			u32 uHandle1 = open(str1,true,false);
			closeAndDelete(uHandle1,NULL);
			CHECK_EQUAL(uHandle1,INVALID_FILE_HANDLE);
		}
	}
}
UNITTEST_SUITE_END

