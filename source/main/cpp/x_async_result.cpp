//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"

#include "xfilesystem\x_async_result.h"
#include "xfilesystem\x_iasync_result.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xiasync_result_null : public xiasync_result
		{
		public:
			virtual bool			isCompleted()																	{ return true; }
			virtual void			waitUntilCompleted()															{ }

			virtual void			clear()																			{ }
			virtual void			hold()																			{ }
			virtual s32				release()																		{ return 1; }
			virtual void			destroy()																		{ }
		};

		static xiasync_result_null	sNullAsyncResultImp;

		//------------------------------------------------------------------------------------------
		xasync_result::xasync_result()
			: mImplementation(&sNullAsyncResultImp)
		{
			mImplementation->hold();
		}

		xasync_result::xasync_result(xiasync_result* imp)
			: mImplementation(imp)
		{
			mImplementation->hold();
		}

		xasync_result::xasync_result(const xasync_result& other)
			: mImplementation(other.mImplementation)
		{
			mImplementation->hold();
		}

		xasync_result::~xasync_result()
		{
			if (mImplementation->release()==0)
				mImplementation->destroy();
		}

		bool xasync_result::isCompleted() const
		{
			return mImplementation->isCompleted();
		}

		void xasync_result::waitUntilCompleted()
		{
			mImplementation->waitUntilCompleted();
		}

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
