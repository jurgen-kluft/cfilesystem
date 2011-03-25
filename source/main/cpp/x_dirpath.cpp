#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_devicealias.h"
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
			: mString(mStringBuffer, sizeof(mStringBuffer))
		{
			mString = str;
			fixSlashes();
		}
		xdirpath::xdirpath(const xdirpath& dirpath)
		{
			mString = dirpath.mString;
		}
		xdirpath::~xdirpath()
		{
		}

		void			xdirpath::clear()										{ mString.clear(); }

		s32				xdirpath::length() const								{ return mString.getLength(); }
		s32				xdirpath::maxLength() const								{ return mString.getMaxLength(); }
		xbool			xdirpath::empty() const									{ return mString.isEmpty(); }
		xbool			xdirpath::isAbsolute() const
		{
			s32 pos = mString.find(":\\");
			return pos >= 0;
		}

		const char*		xdirpath::relative() const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos < 0)
				pos = 0;
			return mString.c_str() + pos;
		}

		bool			xdirpath::getName(char* outName, s32 nameMaxLength) const
		{
			const char* re
		}

		const char*		xdirpath::c_str() const										{ return mString.c_str(); }

		void			xdirpath::setDeviceName(const char* deviceName)
		{
			char deviceNameBuffer[64+1];
			getDeviceName(deviceNameBuffer, sizeof(deviceNameBuffer)-1);
			s32 len = x_strlen(deviceNameBuffer);
			if (deviceName!=NULL)
				mString.replace(0, len, deviceName);
			else if (len > 0)
				mString.remove(0, len + 2);
			fixSlashes(); 
			mString.trimLeft(':');
			mString.trimLeft(sGetSlashChar());
		}

		void			xdirpath::getDeviceName(char* deviceName, s32 deviceNameMaxLength) const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos >= 0)
			{
				xcstring devicePart(deviceName, deviceNameMaxLength+1);
				mString.left(pos, devicePart);
			}
			else
			{
				deviceName[0] = '\0';
			}
		}

		void			xdirpath::setDevicePart(const char* devicePart)
		{
			char devicePartBuffer[64+1];
			getDevicePart(devicePartBuffer, sizeof(devicePartBuffer)-1);
			s32 len = x_strlen(devicePartBuffer);
			if (devicePart!=NULL)
				mString.replace(0, len, devicePart);
			else
				mString.remove(0, len);
			fixSlashes(); 
		}

		void			xdirpath::getDevicePart(char* deviceName, s32 deviceNameMaxLength) const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos >= 0)
			{
				pos += 2;
				xcstring devicePart(deviceName, deviceNameMaxLength+1);
				mString.left(pos, devicePart);
			}
			else
			{
				deviceName[0] = '\0';
			}
		}

		xdirpath&		xdirpath::operator =  ( const xdirpath& path )
		{
			if (path.mString == mString)
				return *this;
			mString = path.mString;
			return *this;
		}

		xdirpath&		xdirpath::operator =  ( const char* str )					{ mString.clear(); mString += str; fixSlashes(); return *this; }
		xdirpath&		xdirpath::operator += ( const char* str )					{ mString += str; fixSlashes(); return *this; }

		bool			xdirpath::operator == ( const xdirpath& rhs) const			{ return (rhs.length() == length()) && x_strCompare(rhs.c_str(), c_str()) == 0; }
		bool			xdirpath::operator != ( const xdirpath& rhs) const			{ return (rhs.length() != length()) || x_strCompare(rhs.c_str(), c_str()) != 0; }
		char			xdirpath::operator [] (s32 index) const						{ return mString[index]; }

		void			xdirpath::fixSlashes()										
		{ 
			const char slash = sGetSlashChar();
			mString.replace(slash=='\\' ? '/' : '\\', slash);
			if (mString.getLength()>0 && mString.lastChar()!=slash)
				mString += slash;
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
