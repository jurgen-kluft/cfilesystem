#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_alias.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"
#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//==============================================================================
		// xfilepath: Device:\\Folder\Folder\Filename.Extension
		//==============================================================================

		xfilepath::xfilepath()
		{
		}
		xfilepath::xfilepath(const char* str)
		{
			mBuffer += str;
			fixSlashes();
		}
		xfilepath::xfilepath(const xstring& str)
			: mBuffer(str)
		{
			fixSlashes();
		}
		xfilepath::xfilepath(const xfilepath& filepath)
			: mBuffer(filepath.mBuffer)
		{
		}
		xfilepath::xfilepath(const xdirpath& dir, const xfilepath& filename)
		{
			mBuffer  = dir.mBuffer;
			mBuffer += filename.mBuffer;
		}
		xfilepath::~xfilepath()
		{
		}

		void			xfilepath::clear()										{ mBuffer.clear(); }

		s32				xfilepath::length() const								{ return mBuffer.getLength(); }
		s32				xfilepath::maxLength() const							{ return mBuffer.getMaxLength(); }
		xbool			xfilepath::empty() const								{ return mBuffer.isEmpty(); }
		xbool			xfilepath::isAbsolute() const
		{
			s32 pos = mBuffer.find(":\\");
			return pos >= 0;
		}

		const char*		xfilepath::extension() const
		{
			s32 pos = mBuffer.rfind(".");
			if (pos >= 0)
				return mBuffer.c_str() + pos;
			return "";
		}

		const char*		xfilepath::relative() const
		{
			s32 pos = mBuffer.find(":\\");
			if (pos < 0)
				pos = 0;
			return mBuffer.c_str() + pos;
		}

		const char*		xfilepath::c_str() const								{ return mBuffer.c_str(); }

		void			xfilepath::setDevicePart(const char* deviceName)
		{
			char devicePartBuffer[64+1];
			getDevicePart(devicePartBuffer, sizeof(devicePartBuffer)-1);
			xcstring devicePart(devicePartBuffer, sizeof(devicePartBuffer));
			mBuffer.replace(0, devicePart.getLength(), deviceName);
			fixSlashes(); 
		}

		void			xfilepath::getDevicePart(char* deviceName, s32 deviceNameMaxLength) const
		{
			s32 pos = mBuffer.find(":\\");
			xcstring devicePart(deviceName, deviceNameMaxLength+1);
			mBuffer.left(pos, devicePart);
		}

		xfilepath&		xfilepath::operator =  ( const xfilepath& path )
		{
			if (path.mBuffer == mBuffer)
				return *this;
			mBuffer = path.mBuffer;
			return *this;
		}

		xfilepath&		xfilepath::operator =  ( const char* str )				{ mBuffer.clear(); mBuffer = mBuffer += str; fixSlashes(); return *this; }
		xfilepath&		xfilepath::operator += ( const char* str )				{ mBuffer += str; fixSlashes(); return *this; }

		bool			xfilepath::operator == ( const xfilepath& rhs) const	{ return (rhs.length() == length()) && x_strCompare(rhs.c_str(), c_str()) == 0; }
		bool			xfilepath::operator != ( const xfilepath& rhs) const	{ return (rhs.length() != length()) || x_strCompare(rhs.c_str(), c_str()) != 0; }

		char			xfilepath::operator [] (s32 index) const				{ return mBuffer[index]; }

		void			xfilepath::fixSlashes()									{ bool UnixStyle = xfilesystem::isPathUNIXStyle(); mBuffer.replace(UnixStyle ? '\\' : '/', UnixStyle ? '/' : '\\'); }

		xfilepath		operator + (xdirpath& dir, xfilepath& filename)
		{
			return xfilepath(dir, filename);
		}

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILEPATH_H__
//==============================================================================
