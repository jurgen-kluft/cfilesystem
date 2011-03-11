//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"

#include "xfilesystem\x_filestream.h"
#include "xfilesystem\x_istream.h"
#include "xfilesystem\x_stream.h"

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

			virtual void			hold()																			{ }
			virtual s32				release()																		{ return 1; }
			virtual void			destroy()																		{ }
		};
		static xiasync_result_null	sNullAsyncResultImp;

		class xstream_imp_empty : public xistream
		{
		public:
			virtual					~xstream_imp_empty(void) {}

			virtual void			hold()																			{ }
			virtual s32				release()																		{ return 1; }
			virtual void			destroy()																		{ }

			virtual bool			canRead() const																	{ return false; }
			virtual bool			canSeek() const																	{ return false; }
			virtual bool			canWrite() const																{ return false; }
			virtual bool			isAsync() const																	{ return false; }
			virtual u64				length() const																	{ return 0; }
			virtual u64				position() const																{ return 0; }
			virtual void			position(u64 Pos)																{ }

			virtual s64				seek(s64 offset, ESeekOrigin origin)											{ return 0; }
			virtual void			close()																			{ }
			virtual void			flush()																			{ }

			virtual void			setLength(s64 length)															{ }

			virtual void			read(xbyte* buffer, s32 offset, s32 count)										{ }
			virtual s32				readByte()																		{ return 0; }
			virtual void			write(const xbyte* buffer, s32 offset, s32 count)								{ }
			virtual void			writeByte(xbyte value)															{ }

			virtual xasync_result	beginRead(xbyte* buffer, s32 offset, s32 count, AsyncCallback callback)			{ return xasync_result(&sNullAsyncResultImp); }
			virtual void			endRead(xasync_result& asyncResult)												{ }
			virtual xasync_result	beginWrite(const xbyte* buffer, s32 offset, s32 count, AsyncCallback callback)	{ return xasync_result(&sNullAsyncResultImp); }
			virtual void			endWrite(xasync_result& asyncResult)											{ }

			virtual void			copyTo(xistream* dst)															{ }
			virtual void			copyTo(xistream* dst, s32 count)												{ }
		};
		static xstream_imp_empty	sNullStreamImp;



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

		//------------------------------------------------------------------------------------------
		xstream::xstream()
			: mImplementation(&sNullStreamImp)
		{
		}

		xstream::xstream(const xstream& other)
			: mImplementation(other.mImplementation)
		{
			mImplementation->hold();
		}

		xstream::xstream(xistream* imp)
			: mImplementation(imp)
		{
			imp->hold();
		}

		bool					xstream::canRead() const
		{
			return mImplementation->canRead();
		}

		bool					xstream::canSeek() const
		{
			return mImplementation->canSeek();
		}

		bool					xstream::canWrite() const
		{
			return mImplementation->canWrite();
		}

		bool					xstream::isAsync() const
		{
			return mImplementation->isAsync();
		}

		u64						xstream::length() const
		{
			return mImplementation->length();
		}

		u64						xstream::position() const
		{
			return mImplementation->position();
		}

		void					xstream::position(u64 Pos)
		{
			mImplementation->position(Pos);
		}


		s64						xstream::seek(s64 offset, ESeekOrigin origin)
		{
			return mImplementation->seek(offset, origin);
		}

		void					xstream::close()
		{
			mImplementation->close();
		}

		void					xstream::flush()
		{
			mImplementation->flush();
		}


		void					xstream::setLength(s64 length)
		{
			mImplementation->setLength(length);
		}


		void					xstream::read(xbyte* buffer, s32 offset, s32 count)
		{
			mImplementation->read(buffer, offset, count);
		}

		s32						xstream::readByte()
		{
			return mImplementation->readByte();
		}

		void					xstream::write(const xbyte* buffer, s32 offset, s32 count)
		{
			mImplementation->write(buffer, offset, count);
		}

		void					xstream::writeByte(xbyte value)
		{
			mImplementation->writeByte(value);
		}


		xasync_result			xstream::beginRead(xbyte* buffer, s32 offset, s32 count, AsyncCallback callback)
		{
			return mImplementation->beginRead(buffer, offset, count, callback);
		}

		void					xstream::endRead(xasync_result& asyncResult)
		{
			mImplementation->endRead(asyncResult);
		}

		xasync_result			xstream::beginWrite(const xbyte* buffer, s32 offset, s32 count, AsyncCallback callback)
		{
			return mImplementation->beginWrite(buffer, offset, count, callback);
		}

		void					xstream::endWrite(xasync_result& asyncResult)
		{
			mImplementation->endWrite(asyncResult);
		}


		void					xstream::copyTo(xstream& dst)
		{
			mImplementation->copyTo(dst.mImplementation);
		}

		void					xstream::copyTo(xstream& dst, s32 count)
		{
			mImplementation->copyTo(dst.mImplementation, count);
		}


	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
