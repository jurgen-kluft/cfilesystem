//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_target.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_stream.h"
#include "xfilesystem/private/x_istream.h"
#include "xfilesystem/private/x_enumerations.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
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

		virtual void			close()																			{ }
		virtual void			flush()																			{ }

		virtual u64				read(xbyte* buffer, u64 offset, u64 count)										{ return 0; }
		virtual u64				readByte(xbyte&)																{ return 0; }
		virtual u64				write(const xbyte* buffer, u64 offset, u64 count)								{ return 0; }
		virtual u64				writeByte(xbyte)																{ return 0; }

		virtual bool			beginRead(xbyte* buffer, u64 offset, u64 count)									{ return false; }
		virtual bool			endRead(bool block)																{ return true; }
		virtual bool			beginWrite(const xbyte* buffer, u64 offset, u64 count, x_asyncio_callback_struct callback)	{ return false; }
		virtual bool			endWrite(bool block)															{ return true; }

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


	u64						xstream::read(xbyte* buffer,  u64 count)
	{
		return mImplementation->read(buffer, count);
	}

	u64						xstream::write(const xbyte* buffer, u64 count)
	{
		return mImplementation->write(buffer, count);
	}

	bool					xstream::beginRead(xbyte* buffer, u64 count)
	{
		return beginRead(buffer, count);
	}

	bool					xstream::beginWrite(const xbyte* buffer, u64 count)
	{
		return beginWrite(buffer, count);
	}



	void					xstream_copy(xstream* src, xstream* dst)
	{
		
	}

	void					xstream::copy(xstream* src, xstream* dst, u64 count)
	{
		
	}


};
