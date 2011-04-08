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

		void			xdirpath::enumLevels(enumerate_delegate<const char*>& folder_enumerator, bool right_to_left) const
		{
			if (isEmpty())
				return;

			const s32 devicePos = mString.find(sGetDeviceEnd());
			const s32 startPos = devicePos>=0 ? devicePos + 2 : 0;

			char folderNameBuffer[128 + 2];
			xcstring folderName(folderNameBuffer, sizeof(folderNameBuffer));

			if (right_to_left == false)
			{
				s32 level = 0;
				const char slash = sGetSlashChar();
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
						folder_enumerator(level, folderName.c_str(), terminate);
						folder_end+=1;
						level++;
					}
					++src;
				}
			}
			else
			{
				s32 level = 0;
				const char slash = sGetSlashChar();
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
						folder_enumerator(level, folderName.c_str(), terminate);
						folder_start-=1;
						level++;
					}
					--src;
				}
			}
		}

		struct folder_counting_enumerator : public enumerate_delegate<const char*>
		{
			s32				mLevels;
							folder_counting_enumerator() : mLevels(0) {}
			virtual void	operator () (s32 level, const char* const& folder, bool& terminate)	{ terminate = false; mLevels++; }
		};

		s32				xdirpath::getLevels() const
		{
			folder_counting_enumerator e;
			enumLevels(e);
			return e.mLevels;
		}

		struct folder_search_enumerator : public enumerate_delegate<const char*>
		{
			const char*		mFolderName;
			s32				mFolderNameLen;

			s32				mLevel;
							
							folder_search_enumerator(const char* folderName, s32 folderNameLen) : mFolderName(folderName), mFolderNameLen(folderNameLen), mLevel(-1) {}

			virtual void	operator () (s32 level, const char* const& folder, bool& terminate)	
			{
				terminate = false; 
			
				const s32 len1 = x_strlen(folder);
				const s32 len2 = mFolderNameLen;
				if (len1 == len2)
				{
					const char* s1 = folder;
					const char* e1 = s1 + len1;
					const char* s2 = mFolderName;
					bool equal = x_strCompareNoCase(s1, len1, s2, len2) == 0;
					if (equal)
					{
						terminate = true;
						mLevel = level;
					}
				}
			}
		};

		s32				xdirpath::getLevelOf(const char* folderName, s32 numChars) const
		{
			if (numChars==-1)
				numChars = x_strlen(folderName);

			folder_search_enumerator e(folderName, numChars);
			enumLevels(e);
			return e.mLevel;
		}

		s32				xdirpath::getLevelOf(const xdirpath& parent) const
		{
			const s32 parentDevicePos = parent.mString.find(sGetDeviceEnd());
			const s32 parentStartPos = parentDevicePos>=0 ? parentDevicePos + 2 : 0;

			const s32 thisDevicePos = mString.find(sGetDeviceEnd());
			const s32 thisStartPos = thisDevicePos>=0 ? thisDevicePos + 2 : 0;

			// Find the overlap

			const char* thisSrc = mString.c_str() + thisStartPos;
			const char* thisEnd = mString.c_str() + mString.getLength();

			const char* parentSrc = parent.mString.c_str() + parentStartPos;
			const char* parentEnd = parent.mString.c_str() + parent.mString.getLength();

			s32 level = -1;
			const char slash = sGetSlashChar();
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

			return terminate ? level : -1;
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
			s32 pos = mString.find(sGetDeviceEnd());
			if (pos < 0)	pos = 0;
			else			pos += 2;
			mString.remove(0, pos);
		}

		void			xdirpath::makeRelativeTo(const xdirpath& parent)
		{
			makeRelative();
			s32 parentDevicePos = parent.mString.find(sGetDeviceEnd());
			if (parentDevicePos>=0)
			{
				parentDevicePos += 2;
				mString.insert(parent.mString.c_str(), parentDevicePos);
			}
			else
			{
				parentDevicePos = 0;
			}

			s32 thisDevicePos = mString.find(sGetDeviceEnd());
			if (thisDevicePos==-1)	thisDevicePos = 0;
			else					thisDevicePos += 2;

			// PARENT:   a\b\c\d\e\f
			// THIS:     c\d\e\f

			// 1. Need to find 'c' from THIS in PARENT
			// 2. Get next level from THIS which is d and see if it comes after the level where 'c' was found in PARENT


			// Find branch in path
			bool has_overlap = false;
			s32 split = 0;
			while (true)
			{
				if (x_isEqualNoCase(mString[thisDevicePos + split], parent.mString[parentDevicePos + split]) == true)
				{
					break;
				}
				++split;
				if ((thisDevicePos + split)>=mString.getLength() || (parentDevicePos + split)>=parent.mString.getLength())
					break;
			}

			while (true)
			{
				if (x_isEqualNoCase(mString[thisDevicePos + split], parent.mString[parentDevicePos + split]) == false)
					break;
				++split;
				if ((thisDevicePos + split)>=mString.getLength() || (parentDevicePos + split)>=parent.mString.getLength())
					break;
			}

			mString.remove(thisDevicePos + split, mString.getLength() - split);
			if ((parentDevicePos + split)<parent.mString.getLength())
				mString += parent.mString.c_str() + parentDevicePos + split;

			fixSlashes();
		}

		void			xdirpath::up()
		{
			if (getLength() > 2)
			{
				s32 lastSlashPos = mString.rfind(sGetSlashChar(), getLength() - 2);
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

		xfiledevice*	xdirpath::getSystem(xcstring& outDirPath) const
		{
			const xdevicealias* alias = getAlias();
			if (alias==NULL)
				return NULL;
			xfiledevice* device = alias->device();
			outDirPath = mString;
			setOrReplaceDevicePart(outDirPath, alias->remap());
			return device;
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
			setOrReplaceDeviceName(mString, inDeviceName);
			fixSlashes(); 
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
			setOrReplaceDevicePart(mString, inDevicePart);
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

		void			xdirpath::setOrReplaceDeviceName(xcstring& ioStr, const char* inDeviceName) const
		{
			s32 len = 0;
			s32 pos = ioStr.find(sGetDeviceEnd());
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

		void			xdirpath::setOrReplaceDevicePart(xcstring& ioStr, const char* inDevicePart) const
		{
			s32 len = ioStr.find(sGetDeviceEnd());
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
