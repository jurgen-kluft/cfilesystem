//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\x_attributes.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	//----------------------- xattributes implementation ---------------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------

	namespace xfilesystem
	{
		enum EAttributes
		{
			FILE_ATTRIBUTE_ARCHIVE	= 1,
			FILE_ATTRIBUTE_READONLY	= 2,
			FILE_ATTRIBUTE_HIDDEN	= 4,
			FILE_ATTRIBUTE_SYSTEM	= 8,
		};

		xattributes::xattributes()
			: mFlags(0)
		{
		}

		xattributes::xattributes(const xattributes& other)
			: mFlags(other.mFlags)
		{
		}

		xattributes::xattributes(bool boArchive, bool boReadonly, bool boHidden, bool boSystem)
			: mFlags(0)
		{
			setArchive(boArchive);
			setReadOnly(boReadonly);
			setHidden(boHidden);
			setSystem(boSystem);
		}

		bool					xattributes::isArchive() const
		{
			return (mFlags & FILE_ATTRIBUTE_ARCHIVE) != 0;
		}

		bool					xattributes::isReadOnly() const
		{
			return (mFlags & FILE_ATTRIBUTE_READONLY) != 0;
		}

		bool					xattributes::isHidden() const
		{
			return (mFlags & FILE_ATTRIBUTE_HIDDEN) != 0;
		}

		bool					xattributes::isSystem() const
		{
			return (mFlags & FILE_ATTRIBUTE_SYSTEM) != 0;
		}

		void					xattributes::setArchive(bool enable)
		{
			if (enable)
				mFlags = mFlags | FILE_ATTRIBUTE_ARCHIVE;
			else
				mFlags = mFlags & ~FILE_ATTRIBUTE_ARCHIVE;
		}

		void					xattributes::setReadOnly(bool enable)
		{
			if (enable)
				mFlags = mFlags | FILE_ATTRIBUTE_READONLY;
			else
				mFlags = mFlags & ~FILE_ATTRIBUTE_READONLY;
		}

		void					xattributes::setHidden(bool enable)
		{
			if (enable)
				mFlags = mFlags | FILE_ATTRIBUTE_HIDDEN;
			else
				mFlags = mFlags & ~FILE_ATTRIBUTE_HIDDEN;
		}

		void					xattributes::setSystem(bool enable)
		{
			if (enable)
				mFlags = mFlags | FILE_ATTRIBUTE_SYSTEM;
			else
				mFlags = mFlags & ~FILE_ATTRIBUTE_SYSTEM;
		}

		xattributes&			xattributes::operator = (const xattributes& other)
		{
			mFlags = other.mFlags;
			return *this;
		}

		bool					xattributes::operator == (const xattributes& other) const
		{
			return mFlags == other.mFlags;
		}

		bool					xattributes::operator != (const xattributes& other) const
		{
			return mFlags != other.mFlags;
		}

	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
