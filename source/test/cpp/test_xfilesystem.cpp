#include "xbase\x_types.h"
#include "xbase\x_allocator.h"

#include "xunittest\xunittest.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_threading.h"
#include "xfilesystem\private\x_filesystem_common.h"

using namespace xcore;

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

extern x_iallocator*					sSystemAllocator;
static TestHeapAllocator				sHeapAllocator(NULL);
static FileSystemIoThreadInterface		sThreadObject;


UNITTEST_SUITE_BEGIN(filesystem_global)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP(){}
		UNITTEST_FIXTURE_TEARDOWN(){}

		// main
		UNITTEST_TEST(setAllocator)
		{
			sHeapAllocator = xcore::TestHeapAllocator(sSystemAllocator);
			xfilesystem::setAllocator(&sHeapAllocator);
			xfilesystem::setAllocator(NULL);
			sHeapAllocator = TestHeapAllocator(NULL);
		}

		UNITTEST_TEST(initialise)
		{
			sHeapAllocator = xcore::TestHeapAllocator(sSystemAllocator);
			xfilesystem::setAllocator(&sHeapAllocator);
			xfilesystem::initialise(10);
			xfilesystem::shutdown();
			xfilesystem::setAllocator(NULL);
			sHeapAllocator = TestHeapAllocator(NULL);
		}

		UNITTEST_TEST(shutdown)
		{
			sHeapAllocator = xcore::TestHeapAllocator(sSystemAllocator);
			xfilesystem::setAllocator(&sHeapAllocator);
			xfilesystem::initialise(20);
			xfilesystem::shutdown();
			xfilesystem::setAllocator(NULL);
			sHeapAllocator = TestHeapAllocator(NULL);
		}

		UNITTEST_TEST(initialiseCommon)
		{
			sHeapAllocator = xcore::TestHeapAllocator(sSystemAllocator);
			xfilesystem::setAllocator(&sHeapAllocator);
			xfilesystem::initialiseCommon(5);
			xfilesystem::shutdownCommon();
			xfilesystem::setAllocator(NULL);
			sHeapAllocator = TestHeapAllocator(NULL);
		}

		UNITTEST_TEST(shutdownCommon)
		{
			sHeapAllocator = xcore::TestHeapAllocator(sSystemAllocator);
			xfilesystem::setAllocator(&sHeapAllocator);
			xfilesystem::initialiseCommon(7);
			xfilesystem::shutdownCommon();
			xfilesystem::setAllocator(NULL);
			sHeapAllocator = TestHeapAllocator(NULL);
		}

		UNITTEST_TEST(setIoThreadInterface)
		{
			xfilesystem::xio_thread* thread1 = &sThreadObject;
			xfilesystem::setIoThreadInterface(thread1);
			xfilesystem::xio_thread* thread2 = xfilesystem::getIoThreadInterface();
			CHECK_NOT_NULL(thread2);
			CHECK_EQUAL(thread1,thread2);
			xfilesystem::setIoThreadInterface(NULL);
		}

		UNITTEST_TEST(getIoThreadInterface)
		{
			xfilesystem::xio_thread* thread1 = &sThreadObject;
			xfilesystem::setIoThreadInterface(thread1);
			xfilesystem::xio_thread* thread2 = xfilesystem::getIoThreadInterface();
			CHECK_NOT_NULL(thread2);
			CHECK_EQUAL(thread1,thread2);
			xfilesystem::setIoThreadInterface(NULL);
		}
	}
}
UNITTEST_SUITE_END


UNITTEST_SUITE_BEGIN(filesystem_init)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		// main 

		UNITTEST_TEST(init)
		{
			sHeapAllocator = xcore::TestHeapAllocator(sSystemAllocator);
			xfilesystem::init(4, &sThreadObject, &sHeapAllocator);
		}
	}
}
UNITTEST_SUITE_END


UNITTEST_SUITE_BEGIN(filesystem_exit)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		// main 

		UNITTEST_TEST(exit)
		{
			xfilesystem::exit();
		}

		UNITTEST_TEST(memory_leaks)
		{
			CHECK_TRUE(sHeapAllocator.mNumAllocations == 0);
			sHeapAllocator = xcore::TestHeapAllocator(NULL);
		}

	}
}
UNITTEST_SUITE_END