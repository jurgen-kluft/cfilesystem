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
		// xdirpath: Device:\\Folder\Folder\Filename.Extension
		//==============================================================================

		static inline char			sGetSlashChar()
		{
			return xfilesystem::isPathUNIXStyle() ? '/' : '\\';
		}
		static inline const char*	sGetDeviceEnd()
		{
			return xfilesystem::isPathUNIXStyle() ? ":/" : ":\\";
		}

		xdirpath::xdirpath()
		{
		}
		xdirpath::xdirpath(const char* str)
		{
			mBuffer += str;
			fixSlashes();
		}
		xdirpath::xdirpath(const xdirpath& dirpath)
		{
			mBuffer = dirpath.mBuffer;
		}
		xdirpath::~xdirpath()
		{
		}

		void			xdirpath::clear()										{ mBuffer.clear(); }

		s32				xdirpath::length() const								{ return mBuffer.getLength(); }
		s32				xdirpath::maxLength() const								{ return mBuffer.getMaxLength(); }
		xbool			xdirpath::empty() const									{ return mBuffer.isEmpty(); }
		xbool			xdirpath::isAbsolute() const
		{
			s32 pos = mBuffer.find(":\\");
			return pos >= 0;
		}

		const char*		xdirpath::relative() const
		{
			s32 pos = mBuffer.find(sGetDeviceEnd());
			if (pos < 0)
				pos = 0;
			return mBuffer.c_str() + pos;
		}

		const char*		xdirpath::c_str() const										{ return mBuffer.c_str(); }

		void			xdirpath::setDevicePart(const char* deviceName)
		{ 
			char devicePartBuffer[64+1];
			getDevicePart(devicePartBuffer, sizeof(devicePartBuffer)-1);
			xcstring devicePart(devicePartBuffer, sizeof(devicePartBuffer));
			mBuffer.replace(0, devicePart.getLength(), deviceName);
			fixSlashes(); 
		}
		void			xdirpath::getDevicePart(char* str, s32 maxStrLen) const		
		{
			xcstring devicePart(str, maxStrLen + 1);
			s32 pos = mBuffer.find(sGetDeviceEnd());
			mBuffer.left(pos, devicePart);
		}

		xdirpath&		xdirpath::operator =  ( const xdirpath& path )
		{
			if (path.mBuffer == mBuffer)
				return *this;
			mBuffer = path.mBuffer;
			return *this;
		}

		xdirpath&		xdirpath::operator =  ( const char* str )					{ mBuffer.clear(); mBuffer += str; fixSlashes(); return *this; }
		xdirpath&		xdirpath::operator += ( const char* str )					{ mBuffer += str; fixSlashes(); return *this; }

		bool			xdirpath::operator == ( const xdirpath& rhs) const			{ return (rhs.length() == length()) && x_strCompare(rhs.c_str(), c_str()) == 0; }
		bool			xdirpath::operator != ( const xdirpath& rhs) const			{ return (rhs.length() != length()) || x_strCompare(rhs.c_str(), c_str()) != 0; }
		char			xdirpath::operator [] (s32 index) const						{ return mBuffer[index]; }

		void			xdirpath::fixSlashes()										
		{ 
			const char slash = sGetSlashChar();
			mBuffer.replace(slash, slash=='\\' ? '/' : '\\');
			if (mBuffer.getLength()>0 && mBuffer.lastChar()!=slash)
				mBuffer += slash;
		}

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
