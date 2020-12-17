#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"
#include "xbase/x_allocator.h"
#include "xbase/x_printf.h"

#include "xunittest/xunittest.h"

#include "filesystem_t/private/x_filedevice.h"
#include "filesystem_t/x_enumerator.h"
#include "filesystem_t/x_filesystem.h"
#include "filesystem_t/x_filepath.h"
#include "filesystem_t/x_dirpath.h"
#include "filesystem_t/x_dirinfo.h"
#include "filesystem_t/x_fileinfo.h"
#include "filesystem_t/x_stream.h"

using namespace xcore;

extern alloc_t* gTestAllocator;

namespace xcore
{
	class utf32_alloc : public runes_alloc_t
	{
	public:
		virtual utf32::runes allocate(s32 len, s32 cap) 
		{
			utf32::prune prunes = (utf32::prune)gTestAllocator->allocate(sizeof(utf32::rune) * cap, sizeof(utf32::rune));
			prunes[cap - 1] = utf32::TERMINATOR;
			utf32::runes runes(prunes, prunes, prunes + cap - 1);
			return runes;
		}
            
		virtual void  deallocate(utf32::runes& slice_t)
		{
			if (slice_t.m_str != nullptr)
			{
				gTestAllocator->deallocate(slice_t.m_str);
			}
			slice_t.m_str = nullptr;
			slice_t.m_end = nullptr;
			slice_t.m_eos = nullptr;
		}
	};
	static utf32_alloc sa;

	template<class T>
	class cstack
	{
		int m_count;
		T	m_items[128];
	
	public:
		void init(int max_count)
		{
			m_count = 0;
		}

		void clear()
		{
			m_count = 0;
		}

		void push(T item)
		{
			m_items[m_count++] = item;
		}

		bool pop(T& item)
		{
			if (m_count == 0)
			{
				item = nullptr;
				return false;
			}
			item = m_items[--m_count];
			return true;
		}
	};

	//------------------------------------------------------------------------------------------
	//---------------------------------- IO Simulated Functions --------------------------------
	//------------------------------------------------------------------------------------------
	//using namespace xfilesystem_test;

	struct TestFile;

	struct TestDir
	{
		ascii::runez<64>			mName;
		datetime_t					mCreationTime;
		datetime_t					mLastAccessTime;
		datetime_t					mLastWriteTime;
		fileattrs_t					mAttr;
	};

	struct TestFile
	{
		ascii::runez<64>			mName;
		fileattrs_t					mAttr;
		datetime_t					mCreationTime;
		datetime_t					mLastAccessTime;
		datetime_t					mLastWriteTime;
		u64							mFileLength;
		u64							mMaxFileLength;
		xbyte						mFileData[4096];
		xbyte						mFileDataEndGuard[16];
		u64							mFilePos;
	};

	struct FileDataCopy
	{
		FileDataCopy(TestFile* to)
		{
			to->mFileLength = 0;
			to->mMaxFileLength = sizeof(to->mFileData);
			init(to);
		}

		FileDataCopy(TestFile* to, const xbyte* from, s32 num_bytes)
		{
			to->mFileLength = num_bytes<sizeof(to->mFileData) ? num_bytes : sizeof(to->mFileData);
			to->mMaxFileLength = sizeof(to->mFileData);
			init(to);
			for (s32 i=0; i<to->mFileLength; ++i)
				to->mFileData[i] = from[i];
		}

		void	init(TestFile* to)
		{
			s32* dst = (s32*)to->mFileData;
			const s32* end_dst = (s32*)(to->mFileData + to->mMaxFileLength - 4);
			while (dst <= end_dst)
				*dst++ = 0;

			for (s32 i=0; i<sizeof(to->mFileDataEndGuard); ++i)
			{
				if (i&1) to->mFileDataEndGuard[i] = 0x0D;
				else to->mFileDataEndGuard[i] = 0xF0;
			}
		}
	};

	// Text data
	static xbyte					sReadme1stData[] = "This is the content of the readme1st textfile.\nIt consists of multiple lines.\nThis is the 3rd line.";
	static xbyte					sAuthorsData[] = "John Carmack\nBill Gates\nSteve Jobs.";
	static xbyte					sTechData[] = "Tech.\nEvery programmer loves tech and gadgets.\nSitting on a cloud with an octacore PC listening to Nirvana.";
	static xbyte					sInstallData[] = "..::Installation Guide::..\n\nInstall Microsoft Developer Studio 2010 together with Power Productivity Tools.\nFor version control use Mercurial.";

	static xbyte					sTexture1[] = { 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67,
													0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59,
													0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7,
													0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1,
													0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05,
													0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83,
													0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29,
													0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
													0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa };
	static xbyte					sTexture2[] = { 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c,
													0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc,
													0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec,
													0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19,
													0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee,
													0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49,
													0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
													0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4,
													0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6,
													0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70,
													0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9,
													0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e,
													0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1,
													0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0,
													0x54, 0xbb, 0x16 };

	static xbyte					sObject1[] = { 0x00, 0x8C, 0x35, 0x04, 0xC8, 0x40, 0xB3, 0x67, 0xD8, 0x42, 0x35, 0x78, 0xF6, 0x2A, 0x02, 0xBE,
													0xF7, 0x1C, 0xCD, 0x9D, 0x98, 0x55, 0x16, 0x3F, 0x81, 0xA4, 0xE5, 0x3E, 0x3D, 0x38, 0x27, 0xEE,
													0x0D, 0x8B, 0xAF, 0xB0, 0xBB, 0xBA, 0xA4, 0xE1, 0xF2, 0xB6, 0x79, 0x92, 0x5B, 0x72, 0xBA, 0xC8,
													0xD7, 0x63, 0xA9, 0x60, 0x17, 0xB1, 0x34, 0xF1, 0xA9, 0xE3, 0x46, 0x67, 0xB8, 0x06, 0x9A, 0xCD,
													0x59, 0x95, 0x10, 0x32, 0x74, 0x15, 0x73, 0xB1, 0x09, 0x43, 0x56, 0xA3, 0x0B, 0xE5, 0x6D, 0x2F,
													0x29, 0xF2, 0xB6, 0x6F, 0x5D, 0xA9, 0x55, 0x19, 0x6A, 0x2E, 0xB0, 0x30, 0x6A, 0x3F, 0xAB, 0x9F,
													0x4F, 0xCE, 0x12, 0x66, 0x28, 0xDE, 0xEB, 0x4C, 0x07, 0x9E, 0x5F, 0x24, 0x47, 0x50, 0x39, 0xB8,
													0x8F, 0x0F, 0xB8, 0x8C, 0x62, 0x8C, 0xC7, 0xA8, 0x30, 0x8C, 0xB3, 0x27, 0xA3, 0x13, 0xBC, 0xB0,
													0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98,
													0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9,
													0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, 0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3,
													0x0a, 0xf7};

	static xbyte					sObject2[] = { 0x0D, 0x55, 0xA9, 0x8B, 0xC6, 0x23, 0x89, 0xF7, 0xD6, 0x6C, 0x31, 0x81, 0xF0, 0x02, 0xEC, 0xD8,
													0xA1, 0xC3, 0xA8, 0x7E, 0x69, 0x71, 0x41, 0x3E, 0xFA, 0x48, 0xD7, 0x3F, 0x89, 0x4C, 0xBA, 0xE6,
													0x4C, 0xE7, 0xBB, 0xBE, 0x4F, 0x05, 0x09, 0x7C, 0x45, 0x00, 0x90, 0xFB, 0xE3, 0x90, 0x82, 0x33,
													0x82, 0x06, 0x04, 0xDE, 0x9B, 0xFA, 0xF6, 0x14, 0xFB, 0x49, 0xA1, 0xE9, 0xAD, 0xAF, 0x21, 0x62,
													0x84, 0x45, 0x3F, 0xB4, 0x6C, 0x8A, 0xFF, 0xE1, 0x8C, 0x5B, 0xC9, 0xBA, 0xD5, 0xB3, 0x48, 0x57,
													0x5A, 0xA8, 0x3F, 0x11, 0x28, 0xD3, 0xB3, 0x2F, 0xAF, 0x32, 0x29, 0xC3, 0xA4, 0x20, 0xC4, 0x03,
													0x89, 0xD4, 0x42, 0x0D, 0xAB, 0x5F, 0x06, 0x7F, 0x57, 0xBE, 0xD1, 0x90, 0x0E, 0x5E, 0x47, 0xE8,
													0xC8, 0xFF, 0xF3, 0x0F, 0xC2, 0xD1, 0x58, 0x9A, 0xEC, 0x66, 0x0B, 0x46, 0x9B, 0x79, 0x33, 0xC1 };

	static xbyte					sTrack1[] = { 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3,
													0x9e, 0x81, 0xf3, 0xd7, 0xfb , 0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f,
													0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb , 0x54,
													0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b,
													0x42, 0xfa, 0xc3, 0x4e , 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24,
													0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25 , 0x72, 0xf8,
													0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d,
													0x65, 0xb6, 0x92 , 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
													0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84 , 0x90, 0xd8, 0xab,
													0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3,
													0x45, 0x06 , 0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1,
													0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b , 0x3a, 0x91, 0x11, 0x41,
													0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6,
													0x73 , 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9,
													0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e , 0x47, 0xf1, 0x1a, 0x71, 0x1d,
													0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b ,
													0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0,
													0xfe, 0x78, 0xcd, 0x5a, 0xf4 , 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07,
													0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f , 0x60,
													0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f,
													0x93, 0xc9, 0x9c, 0xef , 0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5,
													0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61 , 0x17, 0x2b,
													0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55 };

	static xbyte					sTrack2[] = { 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36,
													0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97,
													0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72,
													0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66,
													0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
													0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d,
													0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
													0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61,
													0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
													0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
													0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc,
													0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5,
													0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a,
													0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d,
													0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c,
													0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
													0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4,
													0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
													0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08,
													0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
													0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d,
													0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2,
													0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74 };

	static xbyte					sData10[] = "This is readme.txt which is marked as readonly.";
	static xbyte					sData11[] = {	0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
													0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08 };

	static xbyte					sData12[] = "This is the content of file.txt that is part of the writeable files.";
	static xbyte					sData13[] = {	0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
													0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61,
													0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
													0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40 };

	static TestFile					sFiles[] = 
	{
		{ ascii::runez<64>("textfiles\\readme1st.txt")		, fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("textfiles\\authors.txt")		, fileattrs_t(true ,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("textfiles\\docs\\tech.txt")		, fileattrs_t(false,false,true ,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("textfiles\\tech\\install.txt")	, fileattrs_t(false,false,false,true ), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		 
		{ ascii::runez<64>("binfiles\\texture1.bin")		, fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("binfiles\\texture2.bin")		, fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("binfiles\\objects\\object1.bin"), fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("binfiles\\objects\\object2.bin"), fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("binfiles\\tracks\\track1.bin")	, fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("binfiles\\tracks\\track2.bin")	, fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },

		{ ascii::runez<64>("readonly_files\\readme.txt")	, fileattrs_t(false,true,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("readonly_files\\data.bin")		, fileattrs_t(false,true,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },

		{ ascii::runez<64>("writeable_files\\file.txt")		, fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },
		{ ascii::runez<64>("writeable_files\\file.bin")		, fileattrs_t(false,false,false,false), datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20), 0, 0 },

		// Room for creating files
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__NULL__") }
	};

	static FileDataCopy				sFileData[] =
	{
		FileDataCopy(&sFiles[0], sReadme1stData, sizeof(sReadme1stData)),
		FileDataCopy(&sFiles[1], sAuthorsData, sizeof(sAuthorsData)),
		FileDataCopy(&sFiles[2], sTechData, sizeof(sTechData)),
		FileDataCopy(&sFiles[3], sInstallData, sizeof(sInstallData)),

		FileDataCopy(&sFiles[4], sTexture1, sizeof(sTexture1)),
		FileDataCopy(&sFiles[5], sTexture2, sizeof(sTexture2)),
		FileDataCopy(&sFiles[6], sObject1, sizeof(sObject1)),
		FileDataCopy(&sFiles[7], sObject2, sizeof(sObject2)),
		FileDataCopy(&sFiles[8], sTrack1, sizeof(sTrack1)),
		FileDataCopy(&sFiles[9], sTrack2, sizeof(sTrack2)),

		FileDataCopy(&sFiles[10], sData10, sizeof(sData10)),
		FileDataCopy(&sFiles[11], sData11, sizeof(sData11)),

		FileDataCopy(&sFiles[12], sData12, sizeof(sData12)),
		FileDataCopy(&sFiles[13], sData13, sizeof(sData13)),
	};

	static TestDir	sDirs[] = 
	{
		{ ascii::runez<64>("textfiles")				, datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20) },
		{ ascii::runez<64>("textfiles\\docs")		, datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20) },
		{ ascii::runez<64>("textfiles\\help")		, datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20) },
		{ ascii::runez<64>("binfiles")				, datetime_t(2011, 3, 10, 15, 30, 10), datetime_t(2011, 3, 12, 16, 00, 20), datetime_t(2011, 3, 11, 10, 46, 20) },
		{ ascii::runez<64>("binfiles\\objects")		, datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20) },
		{ ascii::runez<64>("binfiles\\tracks")		, datetime_t(2011, 2, 10, 15, 30, 10), datetime_t(2011, 2, 12, 16, 00, 20), datetime_t(2011, 2, 11, 10, 46, 20) },
		{ ascii::runez<64>("readonly_files")		, datetime_t(2011, 4, 10, 15, 30, 10), datetime_t(2011, 4, 12, 16, 00, 20), datetime_t(2011, 4, 11, 10, 46, 20) },
		{ ascii::runez<64>("writeable_files")		, datetime_t(2011, 4, 10, 15, 30, 10), datetime_t(2011, 4, 12, 16, 00, 20), datetime_t(2011, 4, 11, 10, 46, 20) },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__EMPTY__") },
		{ ascii::runez<64>("__NULL__") }
	};

	namespace xfilesystem_test
	{
		class xfiledevice_TEST : public filedevice_t
		{
			bool					mCanWrite;
		public:
									xfiledevice_TEST()
										: mCanWrite(true)								{ } 
			virtual					~xfiledevice_TEST()									{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return mCanWrite; }

			virtual bool			getDeviceInfo(u64& totalSpace, u64& freeSpace) const;

			virtual bool			openFile(filepath_t const& filepath, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle);
			virtual bool			readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool			writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten);
			virtual bool			closeFile(void* nFileHandle);

			virtual bool			createStream(filepath_t const& szFilename, bool boRead, bool boWrite, stream_t*& strm);
			virtual bool			closeStream(stream_t* strm);

			virtual bool			setLengthOfFile(void* nFileHandle, u64 inLength);
			virtual bool			getLengthOfFile(void* nFileHandle, u64& outLength);

			virtual bool			setFileTime(filepath_t const& filepath, const filetimes_t& time);
			virtual bool			getFileTime(filepath_t const& filepath, filetimes_t& time);
			virtual bool			setFileAttr(filepath_t const& filepath, const fileattrs_t& attr);
			virtual bool			getFileAttr(filepath_t const& filepath, fileattrs_t& attr);

			virtual bool			setFileTime(void* pHandle, filetimes_t const& times);
			virtual bool			getFileTime(void* pHandle, filetimes_t& outTimes);

			virtual bool			hasFile(filepath_t const& filepath);
			virtual bool			moveFile(filepath_t const& filepath, filepath_t const& szToFilename, bool boOverwrite);
			virtual bool			copyFile(filepath_t const& filepath, filepath_t const& szToFilename, bool boOverwrite);
			virtual bool			deleteFile(filepath_t const& filepath);

			virtual bool			hasDir(dirpath_t const& dirpath);
			virtual bool			moveDir(dirpath_t const& dirpath, dirpath_t const& szToDirPath, bool boOverwrite);
			virtual bool			copyDir(dirpath_t const& dirpath, dirpath_t const& szToDirPath, bool boOverwrite);
			virtual bool			createDir(dirpath_t const& dirpath);
			virtual bool			deleteDir(dirpath_t const& dirpath);

			virtual bool			setDirTime(dirpath_t const& dirpath, const filetimes_t& time);
			virtual bool			getDirTime(dirpath_t const& dirpath, filetimes_t& time);
			virtual bool			setDirAttr(dirpath_t const& dirpath, const fileattrs_t& attr);
			virtual bool			getDirAttr(dirpath_t const& dirpath, fileattrs_t& attr);

			virtual bool			enumerate(dirpath_t const& szDirPath, enumerate_delegate_t& _enumerator);

			XCORE_CLASS_PLACEMENT_NEW_DELETE
		};

		static TestDir*		sFindTestDir(const dirpath_t& szDir)
		{
			dirpath_t dp(szDir);

			dp.makeRelative();

			s32 i=0;
			while (true)
			{
				TestDir* testDir = &sDirs[i];
				dirpath_t dp2 = filesystem_t::dirpath(testDir->mName.m_str);

				if (dp == dp2)
					return testDir;
				if (testDir->mName == "__NULL__\\")
					break;

				i++;
			}
			return NULL;
		}

		static TestDir*		sFindEmptyTestDir()
		{
			TestDir* t = sDirs;
			while (true)
			{
				if (t->mName == "__EMPTY__\\")
					return t;
				if (t->mName == "__NULL__\\")
					break;
				t++;
			}
			return NULL;
		}

		static TestFile*	sFindTestFile(filepath_t const& szFilename)
		{
			filepath_t fp(szFilename);			
			fp.makeRelative();

			TestFile* testFile = sFiles;
			while (true)
			{
				filepath_t fp2 = filesystem_t::filepath(testFile->mName.m_str);
				if (fp == fp2)
					return testFile;
				if (testFile->mName == "__NULL__")
					break;
				testFile++;
			}
			return NULL;
		}

		static TestFile*	sNewTestFile(const filepath_t& szFilename, void* data, u32 dataSize)
		{
			TestFile* t = sFiles;
			while (true)
			{
				if (t->mName == "__EMPTY__")
				{
					// filename copy
					// init creation time
					// init last write time
					// data copy
					filesystem_t::to_ascii(szFilename, t->mName);

					t->mFileLength = dataSize;
					t->mMaxFileLength = sizeof(t->mFileData);
					if (t->mFileLength>sizeof(t->mFileData))
						t->mFileLength = sizeof(t->mFileData);

					if (data!=NULL)
						x_memcopy(t->mFileData, data, dataSize);
					else
						x_memclr(t->mFileData, sizeof(t->mFileData));

					return t;
				}

				if (t->mName == "__NULL__")
					break;

				t++;
			}
			return NULL;
		}



		bool xfiledevice_TEST::getDeviceInfo(u64& totalSpace, u64& freeSpace) const
		{
			totalSpace = 16 * 1024 * 1024;
			freeSpace  = 12 * 1024 * 1024;
			return true;
		}

		bool xfiledevice_TEST::hasFile(filepath_t const& szFilename)
		{
			return sFindTestFile(szFilename)!=NULL;
		}
		
		bool xfiledevice_TEST::openFile(filepath_t const& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle)
		{
			if (mode == FileMode_Create || mode == FileMode_CreateNew)
			{
				TestFile* testFile = sFindTestFile(szFilename);
				if (testFile!=NULL)
					return false;

				testFile = sNewTestFile(szFilename, NULL, 0);
				if (testFile==NULL)
					return false;

				nFileHandle = (testFile!=NULL) ? testFile : INVALID_FILE_HANDLE;
				return nFileHandle != INVALID_FILE_HANDLE;
			} 
			else 
			{
				TestFile* testFile = sFindTestFile(szFilename);
				nFileHandle = (testFile!=NULL) ? testFile : INVALID_FILE_HANDLE;
				return nFileHandle != INVALID_FILE_HANDLE;
			}
		}

		bool xfiledevice_TEST::readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead)
		{
			TestFile* testFile = static_cast<TestFile*>((void*)nFileHandle);
			if (testFile!=NULL)
			{
				if (pos >= testFile->mFileLength)
				{
					outNumBytesRead = 0;
					return true;
				}

				u64 bytesToRead = testFile->mFileLength - pos;
				if (count < bytesToRead)
					bytesToRead = count;

				x_memcopy(buffer, testFile->mFileData + pos, (xcore::s32)bytesToRead);
				outNumBytesRead = bytesToRead;
				return true;
			}
			return false;
		}

		bool xfiledevice_TEST::writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten)
		{
			TestFile* testFile = static_cast<TestFile*>((void*)nFileHandle);
			if (testFile!=NULL)
			{
				u64 bytesToWrite = testFile->mMaxFileLength;
				if (count < bytesToWrite)
					bytesToWrite = count;
				if ((pos + count) > testFile->mFileLength)
					testFile->mFileLength = (pos + count);

				// Clamp against the maximum file size that this device has
				if (testFile->mFileLength > testFile->mMaxFileLength)
				{
					bytesToWrite -= (testFile->mFileLength - testFile->mMaxFileLength);
					testFile->mFileLength = testFile->mMaxFileLength;
				}

				x_memcopy(testFile->mFileData + pos, buffer, (s32)bytesToWrite);
				outNumBytesWritten = bytesToWrite;
				return true;
			}
			return false;
		}

		bool xfiledevice_TEST::moveFile(filepath_t const& szFilename, filepath_t const& szToFilename, bool boOverwrite)
		{
			TestFile* testFile = sFindTestFile(szFilename);
			if (testFile!=NULL)
			{
				filepath_t filePath(szFilename);
//				filePath.removeFirstSlashAndBefore();
				filesystem_t::to_ascii(szFilename, testFile->mName);
				return true;
			}
			return false;
		}

		bool xfiledevice_TEST::copyFile(filepath_t const& szFilename, filepath_t const& szToFilename, bool boOverwrite)
		{
			TestFile* srcTestFile = sFindTestFile(szFilename);
			if (srcTestFile!=NULL)
			{
				TestFile* dstTestFile = sFindTestFile(szFilename);
				if (dstTestFile==NULL)
					dstTestFile = sNewTestFile(szFilename, NULL, 0);

				if (dstTestFile!=NULL)
				{
					x_memcpy(dstTestFile->mFileData, srcTestFile->mFileData, sizeof(dstTestFile->mFileData));
					dstTestFile->mFileLength = srcTestFile->mFileLength;
					return true;
				}
			}
			return false;
		}

		bool xfiledevice_TEST::closeFile(void* nFileHandle)
		{
			return true;
		}

		bool xfiledevice_TEST::deleteFile(filepath_t const& szFilename)
		{
			TestFile* testFile = sFindTestFile(szFilename);
			if (testFile!=NULL)
			{
				testFile->mName = "__EMPTY__";
				return true;
			}
			return false;
		}

		bool xfiledevice_TEST::createStream(filepath_t const& szFilename, bool boRead, bool boWrite, stream_t*& strm)
		{
			strm = nullptr;
			return false;
		}
		
		bool xfiledevice_TEST::closeStream(stream_t* strm)
		{
			return false;
		}

		bool xfiledevice_TEST::setLengthOfFile(void* nFileHandle, u64 inLength)
		{
			TestFile* testFile = static_cast<TestFile*>((void*)nFileHandle);
			if (testFile!=NULL)
			{
				testFile->mFileLength = inLength;
				if (testFile->mFileLength > testFile->mMaxFileLength)
					testFile->mFileLength = testFile->mMaxFileLength;
			}
			return testFile!=NULL;
		}

		bool xfiledevice_TEST::getLengthOfFile(void* nFileHandle, u64& outLength)
		{
			TestFile* testFile = static_cast<TestFile*>((void*)nFileHandle);
			outLength = testFile->mFileLength;
			return true;
		}

		bool xfiledevice_TEST::setFileTime(filepath_t const& szFilename, const filetimes_t& time)
		{
			TestFile* testFile = sFindTestFile(szFilename);
			if (testFile!=NULL)
			{
				 time.getTime(testFile->mCreationTime, testFile->mLastAccessTime, testFile->mLastWriteTime);
			}
			return testFile!=NULL;
		}

		bool xfiledevice_TEST::getFileTime(filepath_t const& szFilename, filetimes_t& outTimes)
		{
			TestFile* testFile = sFindTestFile(szFilename);
			if (testFile!=NULL)
			{
				outTimes.setTime(testFile->mCreationTime, testFile->mLastAccessTime, testFile->mLastWriteTime);
			}
			return testFile!=NULL;
		}

		bool			xfiledevice_TEST::setFileAttr(filepath_t const& szFilename, const fileattrs_t& attr)
		{
			TestFile* testFile = sFindTestFile(szFilename);
			if (testFile!=NULL)
			{
				testFile->mAttr = attr;
				return true;
			}
			return false;
		}

		bool			xfiledevice_TEST::getFileAttr(filepath_t const& szFilename, fileattrs_t& attr)
		{
			TestFile* testFile = sFindTestFile(szFilename);
			if (testFile!=NULL)
			{
				attr = testFile->mAttr;
				return true;
			}
			return false;
		}

		bool xfiledevice_TEST::setFileTime(void* fd, const filetimes_t& time)
		{
			TestFile* testFile = (TestFile*)fd;
			if (testFile!=NULL)
			{
				 time.getTime(testFile->mCreationTime, testFile->mLastAccessTime, testFile->mLastWriteTime);
			}
			return testFile!=NULL;
		}

		bool xfiledevice_TEST::getFileTime(void* fd, filetimes_t& outTimes)
		{
			TestFile* testFile = (TestFile*)fd;
			if (testFile!=NULL)
			{
				outTimes.setTime(testFile->mCreationTime, testFile->mLastAccessTime, testFile->mLastWriteTime);
			}
			return testFile!=NULL;
		}

		bool xfiledevice_TEST::hasDir(dirpath_t const& szDirPath)
		{
			return sFindTestDir(szDirPath)!=NULL;
		}

		bool xfiledevice_TEST::createDir(dirpath_t const& szDirPath) 
		{
			TestDir* testDir = sFindTestDir(szDirPath);
			if (testDir!=NULL)
			{
				ascii::printf(ascii::crunes("test file device -- dir %s already exists!!!\n"), va_t(testDir->mName.m_str));
				return false;
			}

			dirpath_t dp(szDirPath);
			dp.makeRelative();

			TestDir* newDir = sFindEmptyTestDir();
			if (newDir!=NULL)
			{
				filesystem_t::to_ascii(dp, newDir->mName);
				newDir->mCreationTime = datetime_t::sNow();
				newDir->mLastAccessTime = newDir->mCreationTime;
				newDir->mLastWriteTime = newDir->mCreationTime;
			}
			else
			{
				ascii::runez<64> dirname;
				filesystem_t::to_ascii(szDirPath, dirname);
				ascii::printf(ascii::crunes("Failed to create test filesystem dir %s!!! \n"), va_t(dirname.m_str));
			}
			return newDir!=NULL;
		}

		static bool enumerateCopyTestDir(dirpath_t const& szDirPath, bool boSearchSubDirectories, enumerate_delegate_t* enumerator, s32 depth)
		{
			dirpath_t dp(szDirPath);
			dp.makeRelative();

			TestDir* testDir = sDirs;
			bool terminate = false;
			while (!terminate)
			{
				dirpath_t tdp = filesystem_t::dirpath(testDir->mName.m_str);
				if (tdp == dp)
				{
					if (enumerator)
					{
						dirinfo_t* di = new dirinfo_t(tdp);
						terminate = (*enumerator)(0, nullptr, di);
					}
				}
				else if (boSearchSubDirectories)
				{
					s32 level = tdp.getLevelOf(dp);
					if (level > 0)
					{
						if (enumerator)
						{
							dirinfo_t* di = new dirinfo_t(tdp);
							terminate = (*enumerator)(level, nullptr, di);
						}
					}
				}

				if (testDir->mName == ascii::crunes("__NULL__"))
					break;
				testDir++;
			}

			bool terminateFile = false;
			TestFile* testFile = sFiles;
			while(!terminateFile)
			{
				filepath_t fpTemp = filesystem_t::filepath(testFile->mName.m_str);
				dirpath_t dpTemp;
				fpTemp.getDirname(dpTemp);
				if (dpTemp == dp)
				{
					if (enumerator)
					{
						fileinfo_t* fiReuslt = new fileinfo_t(fpTemp);
						terminateFile = (*enumerator)(0,fiReuslt,nullptr);
					}
				}
				else if(boSearchSubDirectories)
				{
					s32 levelFile = dpTemp.getLevelOf(dp);
					if (levelFile > 0)
					{
						if (enumerator)
						{
							fileinfo_t* fiResult = new fileinfo_t(fpTemp);
							terminateFile = (*enumerator)(levelFile,fiResult,nullptr);
						}
					}
				}

				if (testFile->mName == ascii::crunes("__NULL__"))
					break;
				testFile ++;
			}

			return true;
		}

		static void changeDirPath(dirpath_t const& szDirPath,const dirpath_t& szToDirPath,const dirinfo_t* szDirinfo,dirpath_t& outDirPath)
		{
			dirpath_t nDirpath_from(szDirPath);
			dirpath_t nDirpath_to(szToDirPath);
			s32 depth1 = nDirpath_from.getLevels();

			dirpath_t parent,child;
			szDirinfo->getDirpath().split(depth1,parent,child);
			outDirPath = nDirpath_to + child;
		}

		static void changeFilePath(dirpath_t const& szDirPath, const dirpath_t& szToDirPath, const fileinfo_t* szFileinfo, filepath_t& outFilePath)
		{
			dirpath_t nDir;
			szFileinfo->getFilepath().getDirname(nDir);

			dirpath_t nDirpath_from(szDirPath);
			dirpath_t nDirpath_to(szToDirPath);

			filepath_t fileName;
			szFileinfo->getFilepath().getFilename(fileName);
			s32 depth = nDirpath_from.getLevels();

			dirpath_t parent,child,copyDirPath_To;
			nDir.split(depth,parent,child);
			copyDirPath_To = nDirpath_to + child;
			outFilePath = filepath_t(copyDirPath_To,fileName);
		}

		struct enumerate_delegate_dirs_copy_testDir : public enumerate_delegate_t
		{
			cstack<const dirinfo_t* > dirStack;
			cstack<const fileinfo_t* > fileStack;
			enumerate_delegate_dirs_copy_testDir() 
			{
				dirStack.init(MAX_ENUM_SEARCH_DIRS);
				fileStack.init(MAX_ENUM_SEARCH_FILES);
			}

			virtual ~enumerate_delegate_dirs_copy_testDir() { dirStack.clear(); }
			virtual bool operator () (s32 depth, fileinfo_t const* fi, dirinfo_t const* di)
			{
				bool terminate = false;
				if (di != nullptr)
				{
					dirStack.push(di);
				}
				else if (fi != nullptr)
				{
					fileStack.push(fi);
				}
				return terminate;
			}
		};

		struct enumerate_delegate_files_copy_testDir : public enumerate_delegate_t
		{
			virtual bool operator () (s32 depth, fileinfo_t const* fi, dirinfo_t const* di)
			{
				bool terminate = false;
				return terminate;
			}
		};


		bool xfiledevice_TEST::moveDir(dirpath_t const& szDirPath, dirpath_t const& szToDirPath, bool boOverwrite) 
		{
			copyDir(szDirPath, szToDirPath, boOverwrite);
			deleteDir(szDirPath);
			return true;
		}

		bool xfiledevice_TEST::copyDir(dirpath_t const& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite) 
		{
			enumerate_delegate_dirs_copy_testDir copy_enum;
			enumerateCopyTestDir(szDirPath, true, &copy_enum, 0);
			
			const dirinfo_t* dirInfo = NULL;
			while(copy_enum.dirStack.pop(dirInfo))
			{
				dirpath_t copyDirPath_To;
				changeDirPath(szDirPath,szToDirPath,dirInfo,copyDirPath_To);
			
				// nDirinfo_From --------------------->   copyDirPath_To       ( copy dir)
				delete dirInfo;		dirInfo = NULL;
				if (!createDir(copyDirPath_To))
					return false;
			}

			const fileinfo_t* fileInfo = NULL;
			while (copy_enum.fileStack.pop(fileInfo))
			{
				filepath_t copyFilePath_To;

				changeFilePath(szDirPath,szToDirPath,fileInfo,copyFilePath_To);
				//nFileinfo_From --------------------->  copyFilePath_To       (copy file)
				bool copyFile_result = copyFile(fileInfo->getFilepath(), copyFilePath_To, false);

				if (!copyFile_result)
				{
					delete fileInfo;    fileInfo = NULL;
					return false;
				}
				delete fileInfo;	fileInfo = NULL;
			}

			return true;
		}
		bool static deleteDirOnly(dirpath_t const& szDirPath)
		{
			TestDir* testDir = sFindTestDir(szDirPath);
			if (testDir==NULL)
				return false;

			// Need to check if there are dirs and files under this directory
			testDir->mName = "__EMPTY__";
			return true;
		}

		bool xfiledevice_TEST::deleteDir(dirpath_t const& szDirPath) 
		{
			enumerate_delegate_dirs_copy_testDir dirs_copy_enum;
			enumerateCopyTestDir(szDirPath, true, &dirs_copy_enum, 0);

			const fileinfo_t* fileInfo = NULL;
			while (dirs_copy_enum.fileStack.pop(fileInfo))
			{
				bool result_file = deleteFile(fileInfo->getFilepath());
				if (!result_file)
				{
					delete fileInfo;
					fileInfo = NULL;
					return false;
				}
				delete fileInfo;
				fileInfo = NULL;
			}

			const dirinfo_t* dirInfo = NULL;
			while(dirs_copy_enum.dirStack.pop(dirInfo))
			{
				bool result_dir = deleteDirOnly(dirInfo->getDirpath());
				if (!result_dir)
				{
					delete dirInfo;
					dirInfo = NULL;
					return false;
				}
				delete dirInfo;
				dirInfo = NULL;
			}
			return true;
		}

		bool xfiledevice_TEST::setDirTime(dirpath_t const& szDirPath, const filetimes_t& times)
		{
			TestDir* testDir = sFindTestDir(szDirPath);
			if (testDir!=NULL)
			{
				times.getTime(testDir->mCreationTime, testDir->mLastAccessTime, testDir->mLastWriteTime);
			}
			return testDir!=NULL;
		}

		bool xfiledevice_TEST::getDirTime(dirpath_t const& szDirPath, filetimes_t& outTimes) 
		{
			TestDir* testDir = sFindTestDir(szDirPath);
			if (testDir!=NULL)
			{
				outTimes.setTime(testDir->mCreationTime, testDir->mLastAccessTime, testDir->mLastWriteTime);
			}
			return testDir!=NULL;
		}

		bool			xfiledevice_TEST::setDirAttr(dirpath_t const& szDirPath, const fileattrs_t& attr) 
		{
			TestDir* testDir = sFindTestDir(szDirPath);
			if (testDir!=NULL)
			{
				testDir->mAttr = attr;
				return true;
			}
			return false;
		}

		bool			xfiledevice_TEST::getDirAttr(dirpath_t const& szDirPath, fileattrs_t& attr) 
		{
			TestDir* testDir = sFindTestDir(szDirPath);
			if (testDir!=NULL)
			{
				attr = testDir->mAttr;
				return true;
			}
			return false;
		}

		bool xfiledevice_TEST::enumerate(dirpath_t const& szDirPath, enumerate_delegate_t& enumerator) 
		{
			dirpath_t dp(szDirPath);
			dp.makeRelative();

			TestDir* testDir = sDirs;
			bool terminate = false;
			while (!terminate)
			{
				dirpath_t tdp = filesystem_t::dirpath(testDir->mName.m_str);
				if (tdp == dp)
				{
					dirinfo_t di(tdp);
					terminate = (enumerator)(0, nullptr, &di);
				}
				else
				{
					s32 level = tdp.getLevelOf(dp);
					if (level > 0)
					{
						dirinfo_t di(tdp);
						terminate = (enumerator)(level, nullptr, &di);
					}
				}

				if (testDir->mName == ascii::crunes("__NULL__"))
					break;

				testDir++;
			}

			bool terminateFile = false;
			TestFile* testFile = sFiles;
			
			while(!terminateFile)
			{
				filepath_t tfp = filesystem_t::filepath(testFile->mName.m_str);
				dirpath_t dpTemp;
				tfp.getDirname(dpTemp);

				if (dpTemp == dp)
				{
					fileinfo_t fiResult(tfp);
					terminateFile = (enumerator)(0, &fiResult, nullptr);
				}
				else 
				{
					s32 levelFile = dpTemp.getLevelOf(dp);
					if (levelFile > 0)
					{
						fileinfo_t fiResult(tfp);
						terminateFile = (enumerator)(levelFile, &fiResult, nullptr);
					}
				}

				if (testFile->mName == ascii::crunes("__NULL__"))
					break;
				testFile ++;
			}

			return true;
		}


		//==============================================================================
		// END filesystem_t namespace
		//==============================================================================
	};
};

using namespace xfilesystem_test;

UNITTEST_SUITE_BEGIN(xfiledevice_register)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP()
		{
			filesyscfg_t cfg;
			cfg.m_allocator = gTestAllocator;
			cfg.m_max_open_stream = 8;
			filesystem_t::create(cfg);
		}
		
		UNITTEST_FIXTURE_TEARDOWN()
		{
			filesystem_t::destroy();
		}

		static xfiledevice_TEST	sTestFileDevice;

		// main 
		UNITTEST_TEST(register_test_filedevice)
		{
			utf32::runez<32> deviceName;
			deviceName = "TEST:\\";
			CHECK_TRUE(filesystem_t::register_device(deviceName, &sTestFileDevice));
		}

		UNITTEST_TEST(hasFile)
		{
			filepath_t fp = filesystem_t::filepath("TEST:\\textfiles\\docs\\tech.txt");
			fileinfo_t fi(fp);
			CHECK_EQUAL(true, fi.exists());
		}
	}
}
UNITTEST_SUITE_END
