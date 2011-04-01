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
			: mString(mStringBuffer, sizeof(mStringBuffer))
		{
		}
		xdirpath::xdirpath(const char* str)
			: mString(mStringBuffer, sizeof(mStringBuffer), str)
		{
			fixSlashes();
		}
		xdirpath::xdirpath(const xdirpath& dirpath)
			: mString(mStringBuffer, sizeof(mStringBuffer), dirpath.c_str())
		{
			mString = dirpath.mString;
		}
		xdirpath::~xdirpath()
		{
		}

		void			xdirpath::clear()										{ mString.clear(); }

		s32				xdirpath::getLength() const								{ return mString.getLength(); }
		s32				xdirpath::getMaxLength() const							{ return mString.getMaxLength(); }
		bool			xdirpath::isEmpty() const								{ return mString.isEmpty(); }

		s32				xdirpath::getLevels() const
		{
			const s32 devicePos = mString.find(sGetDeviceEnd());
			const s32 startPos = devicePos>=0 ? devicePos + 2 : 0;

			s32 levels = 0;
			const char slash = sGetSlashChar();
			const char* src = c_str() + startPos;
			const char* end_src = c_str() + getLength();
			while (src < end_src)
			{
				if (*src == slash && *(src-1) != slash)
					levels++;
				++src;
			}
			
			if (mString.lastChar() == slash)
				--levels;

			return levels;
		}

		bool			xdirpath::isRoot() const
		{
			if (isRooted())
			{
				// Now determine if we have a folder path
				const s32 pos = mString.find(sGetDeviceEnd()) + 2;
				if (pos == getLength())
					return true;
			}
			return false;
		}

		bool			xdirpath::isRooted() const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			return pos >= 0;
		}

		void			xdirpath::relative(xdirpath& outRelative) const
		{
			outRelative = *this;
			outRelative.makeRelative();
		}

		void			xdirpath::makeRelative()
		{
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos < 0)
				pos = 0;
			mString.remove(0, pos);
		}

		void			xdirpath::up()
		{
			if (getLength() > 2)
			{
				s32 lastSlashPos = mString.rfind(sGetSlashChar(), getLength() - 2);
				if (lastSlashPos > 0)
				{
					s32 oneToLastSlashPos = mString.rfind(sGetSlashChar(), lastSlashPos - 1);
					if (oneToLastSlashPos >= 0)
					{
						// Remove this folder
						if (oneToLastSlashPos < lastSlashPos)
						{
							mString.remove(oneToLastSlashPos, lastSlashPos - oneToLastSlashPos);
							fixSlashes();
						}
					}
				}
			}
		}

		void			xdirpath::down(const char* subDir)
		{
			mString += subDir;
			fixSlashes();
		}

		// e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\; sub=="sub\\folder\\";
		void			xdirpath::split(s32 cnt, xdirpath& parent, xdirpath& subDir) const
		{
			const s32 devicePos = mString.find(sGetDeviceEnd());
			const s32 startPos = devicePos>=0 ? devicePos + 2 : 0;

			s32 levels = 0;
			s32 split_pos = -1;
			const char slash = sGetSlashChar();
			const char* src = c_str() + startPos;
			const char* end_src = c_str() + getLength();
			while (src < end_src)
			{
				if (*src == slash && *(src-1) != slash)
				{
					levels++;
					if (levels == cnt)
						split_pos = (src - c_str());
				}
				++src;
			}

			if (split_pos == -1)
				split_pos = startPos;

			mString.left(split_pos, parent.mString);
			parent.fixSlashes();
			mString.right(mString.getLength() - split_pos, subDir.mString);
			subDir.fixSlashes();
		}

		bool			xdirpath::getName(xcstring& outName) const
		{
			const s32 endPos = mString.lastChar()==sGetSlashChar() ? mString.getLength() - 1 : mString.getLength();
			const s32 slashPos = mString.rfind(sGetSlashChar(), endPos - 1);
			if (slashPos >= 0)
			{
				mString.substring((slashPos+1), outName, endPos - (slashPos+1));
				return true;
			}
			return false;
		}

		bool			xdirpath::hasName(const char* inName) const
		{
			const s32 endPos = mString.lastChar()==sGetSlashChar() ? mString.getLength() - 1 : mString.getLength();
			const s32 slashPos = mString.rfind(sGetSlashChar(), endPos - 1);
			const char* src1 = mString.c_str() + ((slashPos >= 0) ? slashPos + 1 : 0);
			{
				const char* end1 = mString.c_str() + endPos;
				const char* src2 = inName;
				while (*src2 != '\0' && src1<end1)
				{
					if (*src1++ != *src2++)
						return false;
				}
				return src1 == end1;
			}
			return false;
		}

		xfiledevice*	xdirpath::getSystem(xcstring& outDirPath) const
		{
			return createSystemPath(c_str(), outDirPath);
		}

		bool			xdirpath::getRoot(xdirpath& outRootDirPath) const
		{
			const s32 deviceSlashPos = mString.find(sGetDeviceEnd());
			if (deviceSlashPos>0)
			{
				mString.left(deviceSlashPos+2, outRootDirPath.mString);
				return true;
			}
			else
			{
				const char* currdir = "currdir";
				const xdevicealias* alias = xdevicealias::sFind(currdir);
				if (alias!=NULL)
				{
					outRootDirPath.mString = currdir;
					outRootDirPath.mString += sGetDeviceEnd();
					return true;
				}
			}
			return false;
		}

		bool			xdirpath::getParent(xdirpath& outParentDirPath) const
		{
			outParentDirPath = *this;
			outParentDirPath.up();
			return true;
		}

		bool			xdirpath::getSubDir(const char* subDir, xdirpath& outSubDirPath) const
		{
			char subDirPathBuffer[XDIR_MAX_PATH];
			xcstring subDirPath(subDirPathBuffer, sizeof(subDirPathBuffer), subDir);
			

			outSubDirPath = *this;
			outSubDirPath.down(subDir);
			return true;
		}

		const char*		xdirpath::c_str() const										{ return mString.c_str(); }

		void			xdirpath::setDeviceName(const char* inDeviceName)
		{
			char deviceNameBuffer[64+1];
			xcstring deviceName(deviceNameBuffer, sizeof(deviceNameBuffer));
			getDeviceName(deviceName);
			s32 len = deviceName.getLength();
			if (inDeviceName!=NULL)
				mString.replace(0, len, inDeviceName);
			else if (len > 0)
				mString.remove(0, len + 2);
			fixSlashes(); 
			mString.trimLeft(':');
			mString.trimLeft(sGetSlashChar());
		}

		void			xdirpath::getDeviceName(xcstring& outDeviceName) const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos >= 0)
			{
				mString.left(pos, outDeviceName);
			}
			else
			{
				outDeviceName.clear();
			}
		}

		void			xdirpath::setDevicePart(const char* inDevicePart)
		{
			char devicePartBuffer[64+1];
			xcstring devicePart(devicePartBuffer, sizeof(devicePartBuffer));
			s32 len = devicePart.getLength();
			if (inDevicePart!=NULL)
				mString.replace(0, len, inDevicePart);
			else
				mString.remove(0, len);
			fixSlashes(); 
		}

		void			xdirpath::getDevicePart(xcstring& outDevicePart) const
		{
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos >= 0)
			{
				pos += 2;
				mString.left(pos, outDevicePart);
			}
			else
			{
				outDevicePart.clear();
			}
		}

		xdirpath&		xdirpath::operator =  ( const xdirpath& path )
		{
			if (this == &path)
				return *this;
			mString = path.mString;
			return *this;
		}

		xdirpath&		xdirpath::operator =  ( const char* str )
		{
			if (c_str() == str)
				return *this;
			mString.clear();
			mString += str;
			fixSlashes();
			return *this; 
		}

		xdirpath&		xdirpath::operator += ( const char* str )					{ mString += str; fixSlashes(); return *this; }

		bool			xdirpath::operator == ( const xdirpath& rhs) const			{ return (rhs.getLength() == getLength()) && x_strCompare(rhs.c_str(), c_str()) == 0; }
		bool			xdirpath::operator != ( const xdirpath& rhs) const			{ return (rhs.getLength() != getLength()) || x_strCompare(rhs.c_str(), c_str()) != 0; }
		char			xdirpath::operator [] (s32 index) const						{ return mString[index]; }

		void			xdirpath::fixSlashes()										
		{ 
			// Replace incorrect slashes with the correct one
			const char slash = sGetSlashChar();
			mString.replace(slash=='\\' ? '/' : '\\', slash);

			// Remove double slashes like '\\' or '//'
			char doubleSlash[4];
			doubleSlash[0] = slash;
			doubleSlash[1] = slash;
			doubleSlash[2] = '\0';
			doubleSlash[3] = '\0';
			mString.replace(doubleSlash, &doubleSlash[1]);

			// Make sure there is NO slash at the start of the path
			if (mString.getLength()>0 && mString.firstChar()==slash)
				mString.trimLeft(slash);

			// Make sure there is a slash at the end of the path
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
