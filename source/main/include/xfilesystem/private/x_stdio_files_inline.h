
//------------------------------------------------------------------------------
// Author:
//     Jurgen Kluft
// Summary:
//     Manages T (Platform specific file IO structure)
// Description:
//     To reduce memory allocations for xCore initialization and shutdown we now
//     handle a short list of static T objects and a dynamic list of T objects.
// See Also:
//     
//------------------------------------------------------------------------------
namespace xcore
{
	template<class T>
	class xfile_device_files
	{
	public:
		enum EHandleType
		{
			HANDLE_TYPE_STATIC	= 0x30000000,
			HANDLE_TYPE_DYNAMIC = 0xff000000,
			HANDLE_INDEX_MASK   = 0x00ffffff,
		};

		s32						mStaticFileCount;
		xsafe_array<T, 16>		mStaticFiles;

		s32						mDynamicFileCount;
		xarray<T>				mDynamicFiles;

		xfile_device_files()
		{
			mStaticFileCount = 0;
			for (s32 i=0; i<mStaticFiles.getCount(); ++i)
				mStaticFiles[i].unbind();

			mDynamicFileCount = 0;
		}

		void*					create()
		{
			void* handle;
			if (mStaticFileCount < mStaticFiles.getCount())
			{
				s32 index=-1;
				for (s32 i=0; i<mStaticFiles.getCount(); ++i)
				{
					if (mStaticFiles[i].empty())
					{
						mStaticFiles[i].bind();
						index = i;
						break;
					}
				}
				ASSERT(index!=-1);
				handle = (void*)(index | HANDLE_TYPE_STATIC);
				mStaticFileCount++;
			}
			else
			{
				// Find a free entry
				s32 index=-1;
				for (s32 i=0; i<mDynamicFiles.getCount(); ++i)
				{
					if (!mDynamicFiles[i].empty())
					{
						mDynamicFiles[i].bind();
						index = i;
						break;
					}
				}
				if (index == -1)
				{
					mDynamicFiles.append(index);
					index = mDynamicFileCount;
				}

				handle = (void*)(index | HANDLE_TYPE_DYNAMIC);
				mDynamicFileCount++;
			}
			return handle;
		}

		void					erase(void* handle)
		{
			if ((((s32)handle) & HANDLE_TYPE_STATIC) == HANDLE_TYPE_STATIC)
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				mStaticFiles[index].unbind();
				--mStaticFileCount;

			}
			else
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				mDynamicFiles[index].unbind();

				--mDynamicFileCount;
				if (mDynamicFileCount==0)
					mDynamicFiles.kill();
			}
		}

		void					get(void* handle, T& out)
		{
			if ((((s32)handle) & HANDLE_TYPE_STATIC) == HANDLE_TYPE_STATIC)
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				out = mStaticFiles[index];
			}
			else
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				out = mDynamicFiles[index];
			}
		}
		void					getPtr(void* handle, T* &out)
		{
			if ((((s32)handle) & HANDLE_TYPE_STATIC) == HANDLE_TYPE_STATIC)
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				out = &mStaticFiles[index];
			}
			else
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				out = &mDynamicFiles[index];
			}
		}
		void					set(void* handle, const T& file)
		{
			if ((((s32)handle) & HANDLE_TYPE_STATIC) == HANDLE_TYPE_STATIC)
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				mStaticFiles[index] = file;
			}
			else
			{
				s32 index = ((s32)handle) & HANDLE_INDEX_MASK;
				mDynamicFiles[index] = file;
			}
		}

		void					kill()
		{
			mStaticFileCount = 0;
			mDynamicFileCount = 0;
			mDynamicFiles.kill();
		}
	};

};
