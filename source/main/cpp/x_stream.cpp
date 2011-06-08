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
		typedef		u64			xasync_id;

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
			virtual void			setPosition(u64 Pos)															{ }

			virtual u64				seek(s64 offset, ESeekOrigin origin)											{ return 0; }
			virtual void			close()																			{ }
			virtual void			flush()																			{ }

			virtual u64				read(xbyte* buffer, u64 offset, u64 count)										{ return 0; }
			virtual s32				readByte()																		{ return 0; }
			virtual u64				write(const xbyte* buffer, u64 offset, u64 count)								{ return 0; }
			virtual u64				writeByte(xbyte value)															{ return 0; }

			virtual xasync_id		beginRead(xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)			{ return -1; }
			virtual void			endRead(xasync_id& asyncResult)													{ }
			virtual xasync_id		beginWrite(const xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)	{ return -1; }
			virtual void			endWrite(xasync_id& asyncResult)												{ }

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

		void					xstream::setPosition(u64 Pos)
		{
			mImplementation->setPosition(Pos);
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

		s32						xstream::readByte()
		{
			return mImplementation->readByte();
		}

		u64						xstream::write(const xbyte* buffer, u64 offset, u64 count)
		{
			return mImplementation->write(buffer, offset, count);
		}

		u64						xstream::writeByte(xbyte value)
		{
			return mImplementation->writeByte(value);
		}


		xasync_id				xstream::beginRead(xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)
		{
			return mImplementation->beginRead(buffer, offset, count, callback);
		}

		void					xstream::endRead(xasync_id& asyncResult)
		{
			mImplementation->endRead(asyncResult);
		}

		xasync_id				xstream::beginWrite(const xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)
		{
			return mImplementation->beginWrite(buffer, offset, count, callback);
		}

		void					xstream::endWrite(xasync_id& asyncResult)
		{
			mImplementation->endWrite(asyncResult);
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
