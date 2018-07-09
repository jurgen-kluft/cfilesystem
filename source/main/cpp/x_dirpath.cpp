#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_ascii.h"
#include "xbase\x_string_utf.h"

#include "xfilesystem\private\x_devicealias.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_dirpath.h"

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
			return xfilesystem::xfs_common::s_instance()->isPathUNIXStyle() ? '/' : '\\';
		}
		static inline char			sGetOtherSlashChar(char c)
		{
			return c == '\\' ? '/' : '\\';
		}

		static inline const char*	sGetDeviceEnd()
		{
			return xfilesystem::xfs_common::s_instance()->isPathUNIXStyle() ? ":/" : ":\\";
 		}

		xdirpath::xdirpath()
		{
		}
		xdirpath::xdirpath(const char* str)
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

		s32				xdirpath::getLength() const								{ return mString.size(); }
		s32				xdirpath::getMaxLength() const							{ return mString.SIZE; }
		bool			xdirpath::isEmpty() const								{ return mString.is_empty(); }

		class dirpath_iterator
		{
		public:
			dirpath_iterator(xcuchars dirPath)
			void			begin(xuchars& device)
			{

			}

			bool			next(xuchars& folder);

			xcuchars		mDirPath;
			xcuchars		mPart;
		};

		void			xdirpath::enumLevels(enumerate_delegate<xucharz<256>>& folder_enumerator, bool right_to_left) const
		{
			if (isEmpty())
				return;

			xcuchars device(":\\");
			xcuchars::const_iterator afterDeviceIter = mString.find_after(device);
			xcuchars::const_iterator startIter = (afterDeviceIter == mString.end()) ? mString.begin() : afterDeviceIter;

			xucharz<256> folderName;

			if (right_to_left == false)
			{
				s32 level = 0;
				const char slash = '\\';
				const char* src = c_str() + startPos;
				const char* end_src = c_str() + getLength();
				const char* folder_start = NULL;
				const char* folder_end = src;
				bool terminate = false;
				while (src < end_src && !terminate)
				{
					if (*src == slash)
					{
						folder_start = folder_end;
						folder_end = src;

						folderName.clear();
						folderName.insert(folder_start, folder_end - folder_start);
						folder_enumerator(level, folderName, terminate);
						folder_end+=1;
						level++;
					}
					++src;
				}
			}
			else
			{
				s32 level = 0;
				const char slash = '\\';
				const char* src = c_str() + getLength() - 2;
				const char* end_src = c_str() + startPos;
				const char* folder_start = src + 1;
				const char* folder_end = NULL;
				bool terminate = false;
				while (src >= end_src && !terminate)
				{
					if (*src == slash)
					{
						folder_end = folder_start;
						folder_start = src + 1;

						folderName.clear();
						folderName.insert(folder_start, folder_end - folder_start);
						folder_enumerator(level, folderName, terminate);
						folder_start-=1;
						level++;
					}
					--src;
				}
			}
		}

		struct folder_counting_enumerator : public enumerate_delegate<xucharz<256>>
		{
			s32				mLevels;
							folder_counting_enumerator() : mLevels(0) {}
							virtual void operator () (s32 level,const char& folder,bool& terminate) { }
			virtual void	operator () (s32 level, const xucharz<256>& folder, bool& terminate)	{ terminate = false; mLevels++; }
		};

		s32				xdirpath::getLevels() const
		{
			folder_counting_enumerator e;
			enumLevels(e);
			return e.mLevels;
		}

		struct folder_search_enumerator : public enumerate_delegate<xucharz<256>>
		{
			xucharz<256>	mFolderName;
			s32				mLevel;
							
			folder_search_enumerator(const xcuchars& folder) : mFolderName(folder), mLevel(-1) {}

			virtual void operator () (s32 level, const xucharz<256>& folder, bool& terminate)
			{
				terminate = false; 
			
				bool equal = mFolderName == folder;
				if (equal)
				{
					terminate = true;
					mLevel = level;
				}
			}
		};

		s32				xdirpath::getLevelOf(const xcuchars& folderName) const
		{
			folder_search_enumerator e(folderName);
			enumLevels(e);
			return e.mLevel;
		}

		s32				xdirpath::getLevelOf(const xdirpath& parent) const
		{
			// PARENT:   c:\disk
			// THIS:     c:\disk\child
			s32 level = -1;
			xcuchars thisSrc = mString;
			xcuchars parentSrc = parent.mString;

			bool terminate = false;
			bool match = true;
			while (parentSrc < parentEnd && match)
			{
				match = (*parentSrc++ == *thisSrc++);
			}
			terminate = match;
			level = getLevels() - parent.getLevels();
			return terminate ? level : -1;
			//

		}

		bool			xdirpath::isRoot() const
		{
			if (isRooted())
			{
				// Now determine if we have a folder path
				const s32 pos = mString.find(":\\") + 2;
				if (pos == getLength())
					return true;
			}
			return false;
		}

		bool			xdirpath::isRooted() const
		{
			s32 pos = mString.find(":\\");
			return pos >= 0;
		}

		bool			xdirpath::isSubDirOf(const xdirpath& parent) const
		{
			const s32 level = getLevelOf(parent);
			return level!=-1;
		}

		void			xdirpath::relative(xdirpath& outRelative) const
		{
			outRelative = *this;
			outRelative.makeRelative();
		}

		void			xdirpath::makeRelative()
		{
			s32 pos = mString.find(":\\");
			if (pos < 0)	pos = 0;
			else			pos += 2;
			mString.remove(0, pos);
		}

		void			xdirpath::makeRelativeTo(const xdirpath& parent)
		{
			makeRelative();

			s32 parentStartPos = parent.mString.find(":\\");
			if (parentStartPos>=0)	parentStartPos += 2;
			else					parentStartPos = 0;

			s32 thisStartPos = mString.find(":\\");
			if (thisStartPos==-1)	thisStartPos = 0;
			else					thisStartPos += 2;

			// PARENT:   a\b\c\d\e\f
			// THIS:     c\d\e\f

			// 1. Need to find 'c' from THIS in PARENT
			// 2. Get next level from THIS which is d and see if it comes after the level where 'c' was found in PARENT

			// Find the overlap

			const char* thisSrc = mString.c_str() + thisStartPos;
			const char* thisEnd = mString.c_str() + mString.getLength();

			const char* parentSrc = parent.mString.c_str() + parentStartPos;
			const char* parentEnd = parent.mString.c_str() + parent.mString.getLength();

			s32 level = -1;
			const char slash = '\\';
			const char* folder_start = NULL;
			const char* folder_end = parentSrc;
			bool terminate = false;
			while (parentSrc < parentEnd && !terminate)
			{
				if (*parentSrc == slash)
				{
					level++;

					folder_start = folder_end;
					folder_end = parentSrc;

					// See if thisSrc-thisEnd equals the rest of the parentSrc-parentEnd
					bool match = true;
					const char* thisSrc1 = thisSrc;
					const char* parentSrc1 = folder_start;
					while (parentSrc1 < parentEnd && match)
					{
						if (thisSrc1 < thisEnd)
							match = (*thisSrc1++ == *parentSrc1++);
						else
							match = false;
					}
					folder_end+=1;
					terminate = match;
				}
				++parentSrc;
			}
			
			if (terminate)
			{
				mString.insert(parent.mString.c_str(), folder_start - parent.mString.c_str());
				fixSlashes();
			}
			else
			{
				level = -1;
			}

			//return level;
		}

		void			xdirpath::up()
		{
			if (getLength() > 2)
			{
				s32 lastSlashPos = mString.rfind('\\', getLength() - 2);
				if (lastSlashPos > 0)
				{
					// Remove this folder
					lastSlashPos += 1;
					mString.remove(lastSlashPos, getLength() - lastSlashPos);
					fixSlashes();
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
			const s32 devicePos = mString.find(":\\");
			const s32 startPos = devicePos>=0 ? devicePos + 2 : 0;

			s32 levels = 0;
			s32 split_pos = -1;
			const char slash = '\\';
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

		bool			xdirpath::getName(xuchars& outName) const
		{
			const s32 endPos = mString.lastChar()=='\\' ? mString.getLength() - 1 : mString.getLength();
			const s32 slashPos = mString.rfind('\\', endPos - 1);
			if (slashPos >= 0)
			{
				mString.substring((slashPos+1), outName, endPos - (slashPos+1));
				return true;
			}
			return false;
		}

		bool			xdirpath::hasName(const xcuchars& inName) const
		{
			folder_search_enumerator e(inName);
			enumLevels(e);
			return e.mLevel != -1;
		}

		const xdevicealias*	xdirpath::getAlias() const
		{
			const xfilesystem::xdevicealias* alias = xdevicealias::sFind(*this);
			if (alias == NULL)
				alias = xdevicealias::sFind("curdir");
			return alias;
		}

		xfiledevice*	xdirpath::getDevice() const
		{
			const xdevicealias* alias = getAlias();
			xfiledevice* device = alias->device();
			return device;
		}

		xfiledevice*	xdirpath::getSystem(xuchars& outDirPath) const
		{
			const xdevicealias* alias = getAlias();
			if (alias==NULL)
				return NULL;
			xfiledevice* device = alias->device();
			outDirPath = mString;
			setOrReplaceDevicePart(outDirPath, alias->remap());
			
			//if(xfs_common::s_instance()->isPathUNIXStyle())
			//	outDirPath.remove(":");

			outDirPath.replace('\\',sGetSlashChar());
			return device;
		}

		bool			xdirpath::getRoot(xdirpath& outRootDirPath) const
		{
			const s32 deviceSlashPos = mString.find(":\\");
			if (deviceSlashPos>0)
			{
				mString.left(deviceSlashPos+2, outRootDirPath.mString);
				return true;
			}
			else
			{
				const char* curdir = "curdir";
				const xdevicealias* alias = xdevicealias::sFind(curdir);
				if (alias!=NULL)
				{
					outRootDirPath.mString = curdir;
					outRootDirPath.mString += ":\\";
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

		bool			xdirpath::getSubDir(xcuchars const& subDir, xdirpath& outSubDirPath) const
		{
			outSubDirPath = *this;
			outSubDirPath.down(subDir);
			return true;
		}

		const xcuchars&	xdirpath::cchars() const	{ return mString; }

		void			xdirpath::setDeviceName(const xcuchars& inDeviceName)
		{
			setOrReplaceDeviceName(mString, inDeviceName);
			fixSlashes(); 
		}

		bool			xdirpath::getDeviceName(xuchars& outDeviceName) const
		{
			s32 pos = mString.find(":\\");
			if (pos >= 0)
				mString.left(pos, outDeviceName);
			return (pos >= 0);
		}

		void			xdirpath::setDevicePart(const xcuchars& inDevicePart)
		{
			setOrReplaceDevicePart(mString, inDevicePart);
			fixSlashes(); 
		}

		bool			xdirpath::getDevicePart(xuchars& outDevicePart) const
		{
			s32 pos = mString.find(":\\");
			if (pos >= 0)
				mString.left(pos + 2, outDevicePart);
			return (pos >= 0);
		}

		xdirpath&		xdirpath::operator =  ( const xdirpath& path )
		{
			if (this == &path)
				return *this;
			mString = path.mString;
			return *this;
		}

		xdirpath&		xdirpath::operator = (const xcuchars& str )
		{
			mString.clear();
			mString += str;
			fixSlashes();
			return *this; 
		}

		bool			xdirpath::operator == ( const xdirpath& rhs) const			{ return rhs.mString.compare(mString) == 0; }
		bool			xdirpath::operator != ( const xdirpath& rhs) const			{ return rhs.mString.compare(mString) != 0; }

#ifdef TARGET_PS3
		bool			xdirpath::makeRelativeForPS3()
		{
			s32 leftPos = mString.find("\\");
			if (leftPos <= 0)
				return false;
			mString.remove(0,leftPos+1);
			fixSlashes();
			mStringForDevice.clear();
			mStringForDevice = mString;
			return true;
		}
#endif

		void			xdirpath::setOrReplaceDeviceName(xuchars& ioStr, xcuchars const& inDeviceName) const
		{
			s32 len = 0;
			s32 pos = ioStr.find(":\\");
			if (pos >= 0)
				len = pos;

			if (inDeviceName!=NULL)
			{
				if (len>0)
					ioStr.replace(0, len, inDeviceName);
				else
				{
					ioStr.insert(":\\");
					ioStr.insert(inDeviceName);
				}
			}
			else if (len > 0)
			{
				ioStr.remove(0, len + 2);
			}
		}

		void			xdirpath::setOrReplaceDevicePart(xuchars& ioStr, xcuchars const& inDevicePart) const
		{
			s32 len = ioStr.find(":\\");
			if (len>=0)	len+=2;
			else len=0;
			if (inDevicePart!=NULL)
			{
				if (len>0)
					ioStr.replace(0, len, inDevicePart);
				else
					ioStr.insert(inDevicePart);
			}
			else if (len>0)
			{
				ioStr.remove(0, len);
			}
		}

		void			xdirpath::fixSlashes()										
		{ 
			// Replace incorrect slashes with the correct one
			const char slash = sGetSlashChar();
		//	mString.replace(slash=='\\' ? '/' : '\\', slash);
			const char rightSlash = '\\';
			mString.replace('/', rightSlash);
			// Remove double slashes like '\\' or '//'
			char doubleSlash[4];
// 			doubleSlash[0] = slash;
// 			doubleSlash[1] = slash;
			doubleSlash[0] = rightSlash;
			doubleSlash[1] = rightSlash;
			doubleSlash[2] = '\0';
			doubleSlash[3] = '\0';
			mString.replace(doubleSlash, &doubleSlash[1]);

			// Make sure there is NO slash at the start of the path
// 			if (mString.getLength()>0 && mString.firstChar()==slash)
// 				mString.trimLeft(slash);
			if (mString.getLength()>0 && mString.firstChar()==rightSlash)
			{
				mString.trimLeft(rightSlash);
			}

			// Make sure there is a slash at the end of the path
// 			if (mString.getLength()>0 && mString.lastChar()!=slash)
// 				mString += slash;
			if (mString.getLength()>0 && mString.lastChar()!=rightSlash)
			{
				mString += rightSlash;
			}
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
