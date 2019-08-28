#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "xtime/x_datetime.h"

#include "xunittest/xunittest.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_path.h"
#include "xfilesystem/x_stream.h"

using namespace xcore;

extern xcore::xalloc* gTestAllocator;

class utf32_alloc : public xcore::utf32::alloc
{
public:
    virtual utf32::runes allocate(s32 len, s32 cap) 
	{
		utf32::prune prunes = (utf32::prune)gTestAllocator->allocate(sizeof(utf32::rune) * cap, sizeof(utf32::rune));
		prunes[cap - 1] = utf32::TERMINATOR;
		utf32::runes runes(prunes, prunes, prunes + cap - 1);
		return runes;
	}
            
	virtual void  deallocate(utf32::runes& slice)
	{
		if (slice.m_str != nullptr)
		{
			gTestAllocator->deallocate(slice.m_str);
		}
		slice.m_str = nullptr;
		slice.m_end = nullptr;
		slice.m_eos = nullptr;
	}
};
static utf32_alloc sUtf32Alloc;

UNITTEST_SUITE_BEGIN(dirinfo)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		static xdatetime sCreationTime(2011, 2, 10, 15, 30, 10);
		static xdatetime sLastAccessTime(2011, 2, 12, 16, 00, 20);
		static xdatetime sLastWriteTime(2011, 2, 11, 10, 46, 20);


		UNITTEST_TEST(constructor0)
		{
			xdirinfo di;
			CHECK_TRUE(di.getDirpath().isEmpty());
		}

		UNITTEST_TEST(constructor1)
		{
			const char* str = "TEST:\\textfiles\\docs";
			xdirinfo di = xpath::as_dirinfo(str, &sUtf32Alloc);
			CHECK_EQUAL(0, x_strcmp(di.getDirpath().c_str(), "TEST:\\textfiles\\docs\\"));

			const char* str2 = "textfiles\\docs";
			xdirinfo di2(str2);
			CHECK_TRUE(di2.getFullName().c_str(), "textfiles\\docs\\"));
		}

		UNITTEST_TEST(constructor2)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);
			xdirinfo di2(di);
		
			CHECK_EQUAL(0, x_strcmp(di2.getFullName().c_str(), str));
		}

		UNITTEST_TEST(constructor3)
		{
			const char* str2 = "TEST:\\textfiles\\docs\\the_folder\\";
			xdirpath dp2(str2);
			CHECK_TRUE(dp2 == str2);

			xdirinfo di1(dp2);
			CHECK_TRUE(di1 == str2);
		}

		UNITTEST_TEST(getFullName)
		{
			const char* str = "TEST:\\textfiles\\docs\\the_folder\\";
			xdirinfo di(str);
			CHECK_TRUE(di.getFullName() == str);
		}

		UNITTEST_TEST(getName)
		{
			const char* str = "TEST:\\textfiles\\docs\\the_folder\\";
			xdirinfo di(str);

			char nameBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
			xcstring name(nameBuffer, sizeof(nameBuffer));
			di.getName(name);

			CHECK_EQUAL(0, x_strCompareNoCase("the_folder", nameBuffer));
		}

		UNITTEST_TEST(isValid)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isValid());

			const char* str2 = "INVALID:\\xfilesystem_test\\the_folder\\";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isValid());

			const char* str3 = "testfile\\doc";
			xdirinfo di3(str3);
			CHECK_EQUAL(true,di3.isValid());
		}

		UNITTEST_TEST(isRoot)
		{
			const char* str = "TEST:\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isRoot());

			const char* str2 = "";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isRoot());

			const char* str3 = "textfiles\\docs";
			xdirinfo di3(str3);
			CHECK_EQUAL(false, di3.isRoot());

			const char* str4 = "TEST:\\textfiles\\docs";
			xdirinfo di4(str4);
			CHECK_EQUAL(false, di4.isRoot());
		}

		UNITTEST_TEST(isRooted)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.isRooted());

			const char* str2 = "textfiles\\docs\\";
			xdirinfo di2(str2);
			CHECK_EQUAL(false, di2.isRooted());
		}

		UNITTEST_TEST(exists)
		{
			const char* str = "TEST:\\textfiles\\docs\\new_folder\\";
			xdirinfo di(str);
			CHECK_EQUAL(false, di.exists());

			const char* str1 = "INVALID:\\xfilesystem_test\the_folder\\";
			xdirinfo di1(str1);
			CHECK_EQUAL(false, di1.exists());

			const char* str2 = "TEST:\\textfiles\\docs\\";
			xdirinfo di2(str2);
			CHECK_EQUAL(true, di2.exists());
		}

		UNITTEST_TEST(create)
		{
			const char* str = "TEST:\\textfiles\\docs\\new_folder\\";
			xdirinfo di(str);
			CHECK_EQUAL(false, di.exists());
			CHECK_EQUAL(true, di.create());
			CHECK_EQUAL(true, di.exists());
		}

		UNITTEST_TEST(remove)
		{
			const char* str = "TEST:\\textfiles\\docs\\new_folder\\";
			xdirinfo di(str);
			CHECK_EQUAL(true, di.exists());
			CHECK_EQUAL(true, di.remove());
			CHECK_EQUAL(false, di.exists());
		}

		UNITTEST_TEST(getAlias)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			const xdevicealias* a = di.getAlias();
			CHECK_EQUAL(true, a!=NULL);
			CHECK_EQUAL(0, x_strCompare(a->alias(), "TEST"));

			const char* str1 = "L:\\te\\";
			xdirinfo di1(str1);
			const xdevicealias* a1 = di1.getAlias();
			CHECK_NULL(a1);
		}

		UNITTEST_TEST(getRoot)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\";
			xdirinfo di1(str1);

			xdirinfo root;
			CHECK_EQUAL(true, di1.getRoot(root));
			CHECK_EQUAL(true, root == "TEST:\\");

			const char* str2 = "textfiles\\docs\\";
			xdirinfo di2(str2);
			xdirinfo root1;
			CHECK_EQUAL(true, di2.getRoot(root1));
			CHECK_EQUAL(true,root1 == "curdir:\\")
		}
		
		UNITTEST_TEST(getParent)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\";
			xdirinfo di1(str1);

			xdirinfo parent;
			CHECK_EQUAL(true, di1.getParent(parent));
			CHECK_EQUAL(true, parent == "TEST:\\textfiles\\");
		}

		UNITTEST_TEST(getSubdir)
		{
			const char* str1 = "TEST:\\textfiles\\";
			xdirinfo di1(str1);

			xdirinfo sub;
			CHECK_EQUAL(true, di1.getSubdir("docs", sub));
			CHECK_EQUAL(true, sub == "TEST:\\textfiles\\docs\\");
		}

		UNITTEST_TEST(getCreationTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime time;
			CHECK_EQUAL(true, di.getCreationTime(time));
			CHECK_EQUAL(xTRUE, sCreationTime == time);
		}

		UNITTEST_TEST(getLastAccessTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime time;
			CHECK_EQUAL(true, di.getLastAccessTime(time));
			CHECK_EQUAL(xTRUE, sLastAccessTime == time);
		}

		UNITTEST_TEST(getLastWriteTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime time;
			CHECK_EQUAL(true, di.getLastWriteTime(time));
			CHECK_EQUAL(xTRUE, sLastWriteTime == time);
		}

		UNITTEST_TEST(setCreationTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime old_time, set_time(2012, 12, 12, 12, 12, 12), get_time;
			CHECK_EQUAL(true, di.getCreationTime(old_time));
			CHECK_EQUAL(true, di.setCreationTime(set_time));
			CHECK_EQUAL(true, di.getCreationTime(get_time));
			CHECK_EQUAL(xTRUE, set_time == get_time);
			CHECK_EQUAL(true, di.setCreationTime(old_time));
		}

		UNITTEST_TEST(setLastAccessTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime old_time, set_time(2012, 12, 12, 12, 12, 12), get_time;
			CHECK_EQUAL(true, di.getLastAccessTime(old_time));
			CHECK_EQUAL(true, di.setLastAccessTime(set_time));
			CHECK_EQUAL(true, di.getLastAccessTime(get_time));
			CHECK_EQUAL(xTRUE, set_time == get_time);
			CHECK_EQUAL(true, di.setLastAccessTime(old_time));
		}

		UNITTEST_TEST(setLastWriteTime)
		{
			const char* str = "TEST:\\textfiles\\docs\\";
			xdirinfo di(str);

			xdatetime old_time, set_time(2012, 12, 12, 12, 12, 12), get_time;
			CHECK_EQUAL(true, di.getLastWriteTime(old_time));
			CHECK_EQUAL(true, di.setLastWriteTime(set_time));
			CHECK_EQUAL(true, di.getLastWriteTime(get_time));
			CHECK_EQUAL(xTRUE, set_time == get_time);
			CHECK_EQUAL(true, di.setLastWriteTime(old_time));
		}

		UNITTEST_TEST(assignment_operator1)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);
			const char* str2 = "TEST:\\textfiles2";

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles1\\");

			di1 = str2;

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(assignment_operator2)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);
			const char* str2 = "TEST:\\textfiles2";
			xdirinfo di2(str2);

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles1\\");
			CHECK_EQUAL(true, di2 == "TEST:\\textfiles2\\");

			di1 = di2;

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles2\\");
			CHECK_EQUAL(true, di2 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(assignment_operator3)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);
			const char* str2 = "TEST:\\textfiles2";
			xdirpath dp1(str2);

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles1\\");
			CHECK_EQUAL(true, dp1 == "TEST:\\textfiles2\\");

			di1 = dp1;

			CHECK_EQUAL(true, di1 == "TEST:\\textfiles2\\");
			CHECK_EQUAL(true, dp1 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(equality_operator1)
		{
			const char* str1 = "TEST:\\textfiles1\\";
			xdirinfo di1(str1);

			CHECK_EQUAL(true, di1 == str1);
			CHECK_EQUAL(false, di1 == "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(equality_operator2)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(true, di1 == xdirpath(str1));
			CHECK_EQUAL(false, di1 == xdirpath("TEST:\\textfiles2\\"));
		}

		UNITTEST_TEST(equality_operator3)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(true, di1 == xdirinfo(str1));
			CHECK_EQUAL(false, di1 == xdirinfo("TEST:\\textfiles2\\"));
		}

		UNITTEST_TEST(non_equality_operator1)
		{
			const char* str1 = "TEST:\\textfiles1\\";
			xdirinfo di1(str1);

			CHECK_EQUAL(false, di1 != str1);
			CHECK_EQUAL(true, di1 != "TEST:\\textfiles2\\");
		}

		UNITTEST_TEST(non_equality_operator2)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(false, di1 != xdirpath(str1));
			CHECK_EQUAL(true, di1 != xdirpath("TEST:\\textfiles2\\"));
		}

		UNITTEST_TEST(non_equality_operator3)
		{
			const char* str1 = "TEST:\\textfiles1";
			xdirinfo di1(str1);

			CHECK_EQUAL(false, di1 != xdirinfo(str1));
			CHECK_EQUAL(true, di1 != xdirinfo("TEST:\\textfiles2\\"));
		}

		UNITTEST_TEST(sIsValid)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\";
			xdirpath dp1(str1);
			CHECK_TRUE(xdirinfo::sIsValid(dp1));

			const char* str2 = "INVALID:\\xfilesystem_test\\the_folder\\";
			xdirpath dp2(str2);
			CHECK_FALSE(xdirinfo::sIsValid(dp2));

			const char* str3 = "testfile\\doc";
			xdirpath dp3(str3);
			CHECK_TRUE(xdirinfo::sIsValid(dp3));
		}

		UNITTEST_TEST(sExists)
		{
			const char* str1 = "TEST:\\textfiles\\docs\\new_folder\\";
			xdirpath dp1(str1);
			CHECK_FALSE(xdirinfo::sExists(dp1));

			const char* str2 = "INVALID:\\xfilesystem_test\the_folder\\";
			xdirpath dp2(str2);
			CHECK_FALSE(xdirinfo::sExists(dp2));

			const char* str3 = "TEST:\\textfiles\\docs\\";
			xdirpath dp3(str3);
			CHECK_TRUE(xdirinfo::sExists(dp3));
		}

		UNITTEST_TEST(sCreate)
		{
			const char* str = "TEST:\\sCreate\\new_folder\\";
			xdirpath dp(str);
			CHECK_EQUAL(false, xdirinfo::sExists(dp));
			CHECK_EQUAL(true, xdirinfo::sCreate(dp));
			CHECK_EQUAL(true, xdirinfo::sExists(dp));
		}

		UNITTEST_TEST(sDelete)
		{
			const char* str = "TEST:\\sCreate\\new_folder\\";
			xdirpath dp(str);
			CHECK_EQUAL(true, xdirinfo::sExists(dp));
			CHECK_EQUAL(true, xdirinfo::sDelete(dp));
			CHECK_EQUAL(false, xdirinfo::sExists(dp));
		}

		UNITTEST_TEST(sEnumerate)
		{
			struct enumerate_delegate_Files:public enumerate_delegate <xfileinfo>
			{
				xfileinfo info;
				virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)
				{
					if (0 == depth)
					{
						info = inf;
						terminate = true;
					}
				}
			};

			struct enumerate_delegate_Dirs:public enumerate_delegate <xdirinfo>
			{
				xdirinfo info;
				virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)
				{
					if ( 0 == depth)
					{
						info = inf;
						terminate = true;
					}
				}
			};
			const char* str1 = "Test:\\textfiles";
			xdirpath dp1(str1);
			xdirinfo di1("textfiles");
			enumerate_delegate_Files file_enumerator;
			enumerate_delegate_Dirs dir_enumerator;
			xdirinfo::sEnumerate(dp1,file_enumerator,dir_enumerator,false);
			CHECK_TRUE(dir_enumerator.info == di1);

			const char* str2 = "textfiles\\readme1st.txt";
			xfileinfo fi2(str2);
			CHECK_TRUE(file_enumerator.info == fi2);
		}

		UNITTEST_TEST(sEnumerateFiles)
		{
			struct enumerate_delegate_Files:public enumerate_delegate <xfileinfo>
			{
				xfileinfo info;
				virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)
				{
					if (0 == depth)
					{
						info = inf;
						terminate = true;
					}
				}
			};
			const char* str1 = "Test:\\textfiles";
			xdirpath dp1(str1);
			enumerate_delegate_Files file_enumerator;
			xdirinfo::sEnumerateFiles(dp1,file_enumerator,false);
			const char* str2 = "textfiles\\readme1st.txt";
			xfileinfo fi2(str2);
			CHECK_TRUE(file_enumerator.info == fi2);
		}

		UNITTEST_TEST(sEnumerateDirs)
		{
			struct enumerate_delegate_Dirs:public enumerate_delegate <xdirinfo>
			{
				xdirinfo info;
				virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)
				{
					if ( 0 == depth)
					{
						info = inf;
						terminate = true;
					}
				}
			};
			const char* str1 = "Test:\\textfiles";
			xdirpath dp1(str1);
			xdirinfo di1("textfiles");
			enumerate_delegate_Dirs dir_enumerator;
			xdirinfo::sEnumerateDirs(dp1,dir_enumerator,false);
			CHECK_TRUE(dir_enumerator.info == di1);
		}

		UNITTEST_TEST(sSetTime_sGetTime)
		{
			const char* filename = "TEST:\\textfiles";
			xdirpath dp1(filename);
			xdatetime createTime1,lastAccessTime1,lastWriteTime1;
			xdirinfo::sGetTime(dp1,createTime1,lastAccessTime1,lastWriteTime1);

			xdatetime createTime2(2000,1,1,1,1,1);
			xdatetime lastAccessTime2(2001,2,2,2,2,2);
			xdatetime lastWriteTime2(2002,3,3,3,3,3);
			xdirinfo::sSetTime(dp1,createTime2,lastAccessTime2,lastWriteTime2);

			xdatetime createTime3,lastAccessTime3,lastWriteTime3;
			xdirinfo::sGetTime(dp1,createTime3,lastAccessTime3,lastWriteTime3);
			CHECK_TRUE(createTime2 == createTime3);
			CHECK_TRUE(lastAccessTime2 == lastAccessTime3);
			CHECK_TRUE(lastWriteTime2 == lastWriteTime3);
			xdirinfo::sSetTime(dp1,createTime1,lastAccessTime1,lastWriteTime1);
		}

		UNITTEST_TEST(sSetCreationTime_sGetCreationTime)
		{
			const char* filename = "TEST:\\textfiles";
			xdirpath dp1(filename);
			xdatetime createTime1;
			xdirinfo::sGetCreationTime(dp1,createTime1);

			xdatetime createTime2(2000,1,1,1,1,1);
			xdirinfo::sSetCreationTime(dp1,createTime2);

			xdatetime createTime3;
			xdirinfo::sGetCreationTime(dp1,createTime3);
			CHECK_TRUE(createTime3 == createTime2);
			xdirinfo::sSetCreationTime(dp1,createTime1);
		}

		UNITTEST_TEST(sSetLastAccessTime_sGetLastAccessTime)
		{
			const char* filename = "TEST:\\textfiles";
			xdirpath dp1(filename);
			xdatetime lastAccessTime1;
			xdirinfo::sGetLastAccessTime(dp1,lastAccessTime1);

			xdatetime lastAccessTime2(2001,2,2,2,2,2);
			xdirinfo::sSetLastAccessTime(dp1,lastAccessTime2);

			xdatetime lastAccessTime3;
			xdirinfo::sGetLastAccessTime(dp1,lastAccessTime3);
			CHECK_TRUE(lastAccessTime3 == lastAccessTime2);
			xdirinfo::sSetLastAccessTime(dp1,lastAccessTime1);
		}

		UNITTEST_TEST(sSetLastWriteTime_sGetLastWriteTime)
		{
			const char* filename = "TEST:\\textfiles";
			xdirpath dp1(filename);
			xdatetime lastWriteTime1;
			xdirinfo::sGetLastWriteTime(dp1,lastWriteTime1);

			xdatetime lastWriteTime2(2001,2,2,2,2,2);
			xdirinfo::sSetLastWriteTime(dp1,lastWriteTime2);

			xdatetime lastWriteTime3;
			xdirinfo::sGetLastWriteTime(dp1,lastWriteTime3);
			CHECK_TRUE(lastWriteTime3 == lastWriteTime2);
			xdirinfo::sSetLastWriteTime(dp1,lastWriteTime1);
		}

		UNITTEST_TEST(sCopy)
		{

			x_printf("----------Beginning Test sCopy--------------##\n");

			const char* strCopyFrom = "TEST:\\unique\\sCopyFrom\\";
			xdirpath dpCopyFrom(strCopyFrom);
			CHECK_EQUAL(false, xdirinfo::sExists(dpCopyFrom));
			CHECK_EQUAL(true, xdirinfo::sCreate(dpCopyFrom));
			CHECK_EQUAL(true,xdirinfo::sExists(dpCopyFrom));

			const char* str2 = "TEST:\\unique\\sCopyFrom\\childDir\\";
			xdirpath dp2(str2);
			CHECK_EQUAL(false, xdirinfo::sExists(dp2));
			CHECK_EQUAL(true, xdirinfo::sCreate(dp2));
			CHECK_EQUAL(true,xdirinfo::sExists(dp2));

			const char* pathStr1 = "TEST:\\unique\\sCopyFrom\\copy.txt";
			xfilepath fp1(pathStr1);
			xfilestream fs1;
			CHECK_EQUAL(false, xfileinfo::sExists(fp1));
			CHECK_EQUAL(true, xfileinfo::sCreate(fp1,fs1));
			CHECK_EQUAL(true,xfileinfo::sExists(fp1));

			const char* strCopyTo = "TEST:\\unique\\sCopyTo\\";
			xdirpath dpCopyTo(strCopyTo);

			CHECK_TRUE(xdirinfo::sCopy(dpCopyFrom,dpCopyTo,true));

			CHECK_TRUE(xdirinfo::sExists(xdirpath("TEST:\\unique\\sCopyTo\\")));
			CHECK_TRUE(xdirinfo::sExists(xdirpath( "TEST:\\unique\\sCopyTo\\childDir\\")));
			CHECK_TRUE(xfileinfo::sExists(xfilepath("TEST:\\unique\\sCopyTo\\copy.txt")));

			CHECK_TRUE(xdirinfo::sDelete(dpCopyTo));

			CHECK_FALSE(xdirinfo::sExists(xdirpath("TEST:\\unique\\sCopyTo\\")));
			CHECK_FALSE(xdirinfo::sExists(xdirpath( "TEST:\\unique\\sCopyTo\\childDir\\")));
			CHECK_FALSE(xfileinfo::sExists(xfilepath("TEST:\\unique\\sCopyTo\\copy.txt")));
		}

		UNITTEST_TEST(sMove)
		{
			const char* strCopyFrom = "TEST:\\unique\\sCopyFrom\\";
			xdirpath dpCopyFrom(strCopyFrom);
			const char* str2 = "TEST:\\unique\\sCopyFrom\\childDir\\";
			xdirpath dp2(str2);
			const char* pathStr1 = "TEST:\\unique\\sCopyFrom\\copy.txt";
			xfilepath fp1(pathStr1);
			CHECK_EQUAL(true, xdirinfo::sExists(dpCopyFrom));
			CHECK_EQUAL(true, xdirinfo::sExists(dp2));
			CHECK_EQUAL(true, xfileinfo::sExists(fp1));

			const char* strMoveTo = "TEST:\\unique\\sMoveTo\\";
			xdirpath dpMoveTo(strMoveTo);
			CHECK_TRUE(xdirinfo::sMove(dpCopyFrom,dpMoveTo));
			CHECK_TRUE(xdirinfo::sExists(dpMoveTo));
			CHECK_TRUE(xdirinfo::sExists(xdirpath("TEST:\\unique\\sMoveTo\\childDir")));
			CHECK_TRUE(xfileinfo::sExists(xfilepath("TEST:\\unique\\sMoveTo\\copy.txt")));

			CHECK_FALSE(xdirinfo::sExists(dpCopyFrom));
			CHECK_FALSE(xdirinfo::sExists(dp2));
			CHECK_FALSE(xfileinfo::sExists(fp1));
			
			CHECK_TRUE(xdirinfo::sDelete(dpMoveTo));

			CHECK_FALSE(xdirinfo::sExists(dpMoveTo));
			CHECK_FALSE(xdirinfo::sExists(xdirpath("TEST:\\unique\\sMoveTo\\childDir")));
			CHECK_FALSE(xfileinfo::sExists(xfilepath("TEST:\\unique\\sMoveTo\\copy.txt")));
		}
	}
}
UNITTEST_SUITE_END
