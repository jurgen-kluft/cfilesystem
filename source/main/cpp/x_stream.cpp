//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"

#include "xfilesystem\x_async_result.h"
#include "xfilesystem\x_filestream.h"
#include "xfilesystem\x_istream.h"
#include "xfilesystem\x_stream.h"
#include "xfilesystem\private\x_filesystem_constants.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xstream_imp_empty : public xistream
		{
		public:
			virtual					~xstream_imp_empty()															{ }

			virtual void			hold()																			{ }
			virtual s32				release()																		{ return 1; }
			virtual void			destroy()																		{ }

			virtual bool			canRead() const																	{ return false; }
			virtual bool			canSeek() const																	{ return false; }
			virtual bool			canWrite() const																{ return false; }
			virtual bool			isOpen() const																	{ return false; }
			virtual bool			isAsync() const																	{ return false; }
			virtual u64				getLength() const																{ return 0; }
			virtual void			setLength(u64 length)															{ }
			virtual u64				getPosition() const																{ return 0; }
			virtual u64				setPosition(u64 Pos)															{ return 0; }

			virtual u64				seek(s64 offset, ESeekOrigin origin)											{ return 0; }
			virtual void			close()																			{ }
			virtual void			flush()																			{ }

			virtual u64				read(xbyte* buffer, u64 offset, u64 count)										{ return 0; }
			virtual u64				readByte(xbyte&)																{ return 0; }
			virtual u64				write(const xbyte* buffer, u64 offset, u64 count)								{ return 0; }
			virtual u64				writeByte(xbyte)																{ return 0; }

			virtual xasync_result	beginRead(xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)			{ return xasync_result(); }
			virtual void			endRead(xasync_result& asyncResult)												{ }
			virtual xasync_result	beginWrite(const xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)	{ return xasync_result(); }
			virtual void			endWrite(xasync_result& asyncResult)											{ }

			virtual void			copyTo(xistream* dst)															{ }
			virtual void			copyTo(xistream* dst, u64 count)												{ }
		};

		static xstream_imp_empty	sNullStreamImp;

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

		xstream::~xstream()
		{
			if (mImplementation->release() == 0)
				mImplementation->destroy();
			mImplementation = 0;
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

		bool					xstream::isOpen() const
		{
			return mImplementation->isOpen();
		}

		bool					xstream::isAsync() const
		{
			return mImplementation->isAsync();
		}

		u64						xstream::getLength() const
		{
			return mImplementation->getLength();
		}

		void					xstream::setLength(u64 length)
		{
			mImplementation->setLength(length);
		}

		u64						xstream::getPosition() const
		{
			return mImplementation->getPosition();
		}

		u64						xstream::setPosition(u64 Pos)
		{
			return mImplementation->setPosition(Pos);
		}


		u64						xstream::seek(s64 offset, ESeekOrigin origin)
		{
			return mImplementation->seek(offset, origin);
		}

		void					xstream::close()
		{
			mImplementation->close();

			if (mImplementation->release() == 0)
				mImplementation->destroy();

			mImplementation = &sNullStreamImp;
		}

		void					xstream::flush()
		{
			mImplementation->flush();
		}


		u64						xstream::read(xbyte* buffer, u64 offset, u64 count)
		{
			return mImplementation->read(buffer, offset, count);
		}

		u64						xstream::readByte(xbyte& outByte)
		{
			return mImplementation->readByte(outByte);
		}

		u64						xstream::write(const xbyte* buffer, u64 offset, u64 count)
		{
			return mImplementation->write(buffer, offset, count);
		}

		u64						xstream::writeByte(xbyte value)
		{
			return mImplementation->writeByte(value);
		}


		xasync_result			xstream::beginRead(xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)
		{
			return mImplementation->beginRead(buffer, offset, count, callback);
		}

		bool					xstream::endRead(xasync_result& asyncResult, bool block)
		{
			mImplementation->endRead(asyncResult);
			return true;
		}

		xasync_result			xstream::beginWrite(const xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)
		{
			return mImplementation->beginWrite(buffer, offset, count, callback);
		}

		bool					xstream::endWrite(xasync_result& asyncResult, bool block)
		{
			mImplementation->endWrite(asyncResult);
			return true;
		}


		void					xstream::copyTo(xstream& dst)
		{
			mImplementation->copyTo(dst.mImplementation);
		}

		void					xstream::copyTo(xstream& dst, u64 count)
		{
			mImplementation->copyTo(dst.mImplementation, count);
		}


	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
