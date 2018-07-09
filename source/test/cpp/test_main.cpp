#include "xbase/x_target.h"
#include "xbase/x_types.h"
#include "xbase/x_allocator.h"
#include "xbase/x_console.h"

#include "xtime/x_time.h"
#include "xfilesystem/x_filesystem.h"
#include "xunittest/xunittest.h"
#include "xfilesystem/x_threading.h"

using namespace xcore;

UNITTEST_SUITE_LIST(xFileUnitTest);

UNITTEST_SUITE_DECLARE(xFileUnitTest, xfiledevice_register);

UNITTEST_SUITE_DECLARE(xFileUnitTest, dirpath);
UNITTEST_SUITE_DECLARE(xFileUnitTest, filepath);
UNITTEST_SUITE_DECLARE(xFileUnitTest, dirinfo);
UNITTEST_SUITE_DECLARE(xFileUnitTest, fileinfo);
UNITTEST_SUITE_DECLARE(xFileUnitTest, filestream);
UNITTEST_SUITE_DECLARE(xFileUnitTest, filesystem_common);


class UnitTestAllocator : public UnitTest::Allocator
{
public:
	xcore::x_iallocator*	mAllocator;
	int						mNumAllocations;

	UnitTestAllocator(xcore::x_iallocator* allocator)
		: mNumAllocations(0)
	{
		mAllocator = allocator;
	}

	void*	Allocate(int size)
	{
		++mNumAllocations;
		return mAllocator->allocate(size, 4);
	}
	void	Deallocate(void* ptr)
	{
		--mNumAllocations;
		mAllocator->deallocate(ptr);
	}
};


xcore::x_iallocator* sSystemAllocator = NULL;


namespace xcore
{
	class TestHeapAllocator : public x_iallocator
	{
	public:
		TestHeapAllocator(xcore::x_iallocator* allocator)
			: mAllocator(allocator)
			, mNumAllocations(0)
		{
		}

		xcore::x_iallocator*	mAllocator;
		s32						mNumAllocations;


		const char*	name() const
		{
			return "xstring unittest test heap allocator";
		}

		void*		allocate(u32 size, u32 alignment)
		{
			++mNumAllocations;
			return mAllocator->allocate(size, alignment);
		}

		void*		reallocate(void* mem, u32 size, u32 alignment)
		{
			return mAllocator->reallocate(mem, size, alignment);
		}

		void		deallocate(void* mem)
		{
			--mNumAllocations;
			mAllocator->deallocate(mem);
		}

		void		release()
		{
		}

	};
};

class FileSystemIoThreadInterface : public xfilesystem::xio_thread
{
public:
	virtual void		sleep(u32 ms)
	{
	}

	virtual bool		loop() const 
	{ 
		return false; 
	}

	virtual void		wait()
	{
	}

	virtual void		signal()
	{
		xfilesystem::doIO(this);
	}

};


static FileSystemIoThreadInterface		sThreadObject;



bool gRunUnitTest(UnitTest::TestReporter& reporter)
{
	x_TimeInit ();
	sSystemAllocator = xcore::gCreateSystemAllocator();

	UnitTestAllocator unittestAllocator( sSystemAllocator );
	UnitTest::SetAllocator(&unittestAllocator);
	TestHeapAllocator		heapAllocator(sSystemAllocator);

	xcore::xconsole::addDefault();

	xfilesystem::init(20, &sThreadObject, &heapAllocator);

	int r = UNITTEST_SUITE_RUN(reporter, xFileUnitTest);
	if (unittestAllocator.mNumAllocations!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "xunittest", "memory leaks detected!");
		r = -1;
	}

	xfilesystem::exit();

	sSystemAllocator->release();

	x_TimeExit ();
	return r==0;
}