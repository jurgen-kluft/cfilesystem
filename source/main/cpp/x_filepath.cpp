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
		static inline char			sGetSlashChar()
		{
			return xfilesystem::isPathUNIXStyle() ? '/' : '\\';
		}
		static inline const char*	sGetDeviceEnd()
		{
			return xfilesystem::isPathUNIXStyle() ? ":/" : ":\\";
		}

		//==============================================================================
		// xfilepath: Device:\\Folder\Folder\Filename.Extension
		//==============================================================================

		xfilepath::xfilepath()
			: mString(mStringBuffer, sizeof(mStringBuffer))
		{
		}
		xfilepath::xfilepath(const char* str)
			: mString(mStringBuffer, sizeof(mStringBuffer))
		{
			mString += str;
			fixSlashes();
		}
		xfilepath::xfilepath(const xfilepath& filepath)
			: mString(mStringBuffer, sizeof(mStringBuffer))
		{
			mString += filepath.mString;
		}
		xfilepath::xfilepath(const xdirpath& dir, const xfilepath& filename)
			: mString(mStringBuffer, sizeof(mStringBuffer))
		{
			mString  = dir.mString;
			mString += filename.relative();
		}
		xfilepath::~xfilepath()
		{
		}

		void			xfilepath::clear()										{ mString.clear(); }

		s32				xfilepath::length() const								{ return mString.getLength(); }
		s32				xfilepath::maxLength() const							{ return mString.getMaxLength(); }
		xbool			xfilepath::empty() const								{ return mString.isEmpty(); }
		xbool			xfilepath::isAbsolute() const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			return pos >= 0;
		}

		const char*		xfilepath::extension() const
		{
			s32 pos = mString.rfind(".");
			if (pos >= 0)
				return mString.c_str() + pos;
			return "";
		}

		const char*		xfilepath::relative() const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos < 0)
				pos = 0;
			return mString.c_str() + pos + 2;
		}

		const char*		xfilepath::c_str() const								{ return mString.c_str(); }

		void			xfilepath::setDeviceName(const char* deviceName)
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

		void			xfilepath::getDeviceName(char* deviceName, s32 deviceNameMaxLength) const
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

		void			xfilepath::setDevicePart(const char* devicePart)
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

		void			xfilepath::getDevicePart(char* deviceName, s32 deviceNameMaxLength) const
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

		xfilepath&		xfilepath::operator =  ( const xfilepath& path )
		{
			if (path.mString == mString)
				return *this;
			mString = path.mString;
			return *this;
		}

		xfilepath&		xfilepath::operator =  ( const char* str )				{ mString.clear(); mString += str; fixSlashes(); return *this; }
		xfilepath&		xfilepath::operator += ( const char* str )				{ mString += str; fixSlashes(); return *this; }

		bool			xfilepath::operator == ( const xfilepath& rhs) const	{ return (rhs.length() == length()) && x_strCompare(rhs.c_str(), c_str()) == 0; }
		bool			xfilepath::operator != ( const xfilepath& rhs) const	{ return (rhs.length() != length()) || x_strCompare(rhs.c_str(), c_str()) != 0; }

		char			xfilepath::operator [] (s32 index) const				{ return mString[index]; }

		void			xfilepath::fixSlashes()									
		{ 
			const char slash = sGetSlashChar();
			mString.replace(slash=='\\' ? '/' : '\\', slash);
			mString.trimDelimiters(slash, slash);
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
