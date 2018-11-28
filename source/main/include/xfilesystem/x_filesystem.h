#ifndef __X_FILESYSTEM_H__
#define __X_FILESYSTEM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"

namespace xcore
{
	///< Forward declares
	class xalloc;

	class xfilesystem;
	class xio_thread;

	///< Initialization
	extern xfilesystem*		create_fs( u32 max_open_streams, xio_thread* io_thread, xalloc* allocator );
	extern void				destroy_fs( xfilesystem* );

	///< doIO; user has to call this from either the main thread or an Io thread.
	///< This call will block the calling thread and it will stay in a do-while
	///< until threading->loop() is false.
	extern void				doIO	( xio_thread* );

	class xfile;
	class xinfo;
	class xwriter;
	class xreader;
	class xfilepath;

	class xfilesystem
	{
	public:
		xfile*		open(xfilepath const& filename, EFileMode mode);
		xwriter*	writer(xfile*);
		xreader*	reader(xfile*);
		
		void		close(xfile*);
		void		close(xinfo*);
		void		close(xreader*);
		void		close(xwriter*);
		void		close(xstream*);

		xinfo*		info(xfilepath const& path);
		bool		exists(xinfo*);
		s64			size(xinfo*);
		xfile*		open(xinfo*, emode mode);
		void		rename(xinfo*, xfilepath const&);
		void		move(xinfo* src, xinfo* dst);
		void		copy(xinfo* src, xinfo* dst);
		void		remove(xinfo*);

		s32			read(reader*, xbuffer&);
		s32			write(writer*, xcbuffer const&);

		void		read_async(reader*, xbuffer&);
		s32			wait_async(reader*);

		bool		register_alias (xstring const& alias, xstring const& target);

		xstream*	filestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op);

		xfs_imp*	mInstance;
	};

};

#endif	// __X_FILESYSTEM_H__
