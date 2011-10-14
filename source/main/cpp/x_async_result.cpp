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
			virtual					~xiasync_result_null()															{ }

			virtual bool			checkForCompletion()															{ return true; }
			virtual void			waitForCompletion()																{ }

			virtual u64				getResult() const																{ return 0; }

			virtual void			clear()																			{ }
			virtual s32				hold()																			{ return 0; }
			virtual s32				release()																		{ return 1; }
			virtual void			destroy()																		{ }
		};

		static xiasync_result_null	sNullAsyncResultImp;

		//------------------------------------------------------------------------------------------
		xasync_result::xasync_result()
			: mImplementation(&sNullAsyncResultImp)
			, mResult(0)
		{
		}

		xasync_result::xasync_result(xiasync_result* imp)
			: mImplementation(imp)
		{
			mResult = imp->getResult();
			if (mImplementation->hold()==0)
				mImplementation = &sNullAsyncResultImp;
		}

		xasync_result::xasync_result(const xasync_result& other)
			: mImplementation(other.mImplementation)
			, mResult(other.mResult)
		{
			if (mImplementation->hold()==0)
				mImplementation = &sNullAsyncResultImp;
		}

		xasync_result::~xasync_result()
		{
			if (mImplementation->release()==0)
				mImplementation->destroy();
			mImplementation = 0;
		}

		bool xasync_result::checkForCompletion()
		{
		//	if (mImplementation==&sNullAsyncResultImp)
		//		return true;

			bool _completed = mImplementation->checkForCompletion();
			if (_completed)
			{
				mResult = mImplementation->getResult();
				if (mImplementation->release()==0)
					mImplementation->destroy();
				mImplementation = &sNullAsyncResultImp;
			}
			return _completed;
		}

		void xasync_result::waitForCompletion()
		{
			if (mImplementation==&sNullAsyncResultImp)
				return;

			mImplementation->waitForCompletion();
			mResult = mImplementation->getResult();
			if (mImplementation->release()==0)
				mImplementation->destroy();
			mImplementation = &sNullAsyncResultImp;
		}

		u64	xasync_result::getResult() const
		{
			return mResult;
		}

		xasync_result&	xasync_result::operator =  (const xasync_result& other)
		{
			if (mImplementation->release()==0)
				mImplementation->destroy();

			mResult = other.mResult;
			if (other.mImplementation->hold()==0)
				mImplementation = &sNullAsyncResultImp;
			else
				mImplementation = other.mImplementation;

			return *this;
		}

		bool	xasync_result::operator == (const xasync_result& other) const
		{
			return mImplementation == other.mImplementation;
		}

		bool	xasync_result::operator != (const xasync_result& other) const
		{
			return mImplementation != other.mImplementation;
		}

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
