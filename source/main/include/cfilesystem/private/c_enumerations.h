#ifndef __C_FILESYSTEM_ENUMERATIONS_H__
#define __C_FILESYSTEM_ENUMERATIONS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"

namespace ncore
{
    namespace nfs
    {
        typedef u32 async_id_t;

        namespace ESyncMode
        {
            enum EEnum
            {
                WAIT   = 0x00,
                NOWAIT = 0x01,
            };
        }

        namespace ESettings
        {
            enum EEnum
            {
                MAX_PATH      = 256,
                MEM_ALIGNMENT = 128,
            };
        }

        extern void* INVALID_FILE_HANDLE;
        extern void* PENDING_FILE_HANDLE;
        extern void* INVALID_DIR_HANDLE;

        namespace EStreamCaps
        {
            enum EEnumValue
            {
                Value_None  = 0x0000,
                Value_Async = 0x0001,
                Value_Write = 0x0002,
                Value_Seek  = 0x0004,
            };

            struct Enum
            {
                Enum() : value(Value_None) {}
                Enum(EEnumValue value) : value(value) {}
                s8 value;

                inline bool CanAsync() const { return (value&Value_Async) == Value_Async; }
                inline bool CanWrite() const { return (value&Value_Write) == Value_Write; }
                inline bool CanSeek() const { return (value&Value_Seek) == Value_Seek; }

                const char* ToString() const;

                inline Enum(EEnumValue value) : value(value) {}
                operator EEnumValue() const { return (EEnumValue)value; }
            };

            inline Enum None() { return Enum(Value_None); }
        }

        namespace ESeekOrigin
        {
            enum EEnumValue
            {
                Value_Begin,   // Specifies the beginning of a stream.
                Value_Current, // Specifies the current position within a stream.
                Value_End,     // Specifies the end of a stream.
            };

            struct Enum
            {
                s8 value;

                inline bool IsBegin() const { return value == Value_Begin; }
                inline bool IsCurrent() const { return value == Value_Current; }
                inline bool IsEnd() const { return value == Value_End; }

                const char* ToString() const;

                inline Enum(EEnumValue value) : value(value) {}
                operator EEnumValue() const { return (EEnumValue)value; }
            };

            inline Enum Begin() { return Enum(Value_Begin); }
            inline Enum Current() { return Enum(Value_Current); }
            inline Enum End() { return Enum(Value_End); }
        }; // namespace ESeekOrigin

        namespace EFileMode
        {
            enum EEnumValue
            {
                Value_CreateNew,    // Specifies that the operating system should create a new file. If the file already exists it will do nothing.
                Value_Create,       // Specifies that the operating system should create a new file. If the file already exists, it will be overwritten. EFileMode.Create is equivalent to requesting that if the file does not exist, use CreateNew; otherwise,
                                    // use Truncate.
                Value_Open,         // Specifies that the operating system should open an existing file. The ability to open the file is dependent on the value specified by EFileAccess. Nothing is done if the file does not exist.
                Value_OpenOrCreate, // Specifies that the operating system should open a file if it exists; otherwise, a new file should be created.
                Value_Truncate,     // Specifies that the operating system should open an existing file. Once opened, the file should be truncated so that its size is zero bytes.
                Value_Append        // Opens the file if it exists and seeks to the end of the file, or creates a new file. Attempting to seek to a position before the end of the file will do nothing and any attempt to read fails.
            };

            struct Enum
            {
                s8 value;

                inline bool IsCreateNew() const { return value == Value_CreateNew; }
                inline bool IsCreate() const { return value == Value_Create; }
                inline bool IsOpen() const { return value == Value_Open; }
                inline bool IsOpenOrCreate() const { return value == Value_OpenOrCreate; }
                inline bool IsTruncate() const { return value == Value_Truncate; }
                inline bool IsAppend() const { return value == Value_Append; }

                const char* ToString() const;

                inline Enum(EEnumValue value) : value(value) {}
                operator EEnumValue() const { return (EEnumValue)value; }
            };

            inline bool CreateNew() { return Enum(Value_CreateNew); }
            inline bool Create() { return Enum(Value_Create); }
            inline bool Open() { return Enum(Value_Open); }
            inline bool OpenOrCreate() { return Enum(Value_OpenOrCreate); }
            inline bool Truncate() { return Enum(Value_Truncate); }
            inline bool Append() { return Enum(Value_Append); }

        } // namespace EFileMode

        namespace EFileAccess
        {
            enum EEnumValue
            {
                Value_Read      = 0x01, // Specifies that only reading is allowed
                Value_Write     = 0x02, // Specifies that only writing is allowed
                Value_ReadWrite = 0x03, // Specifies that both reading and writing are allowed
            };

            struct Enum
            {
                s8 value;

                inline bool IsRead() const { return value == Value_Read; }
                inline bool IsWrite() const { return value == Value_Write; }
                inline bool IsReadWrite() const { return value == Value_ReadWrite; }

                const char* ToString() const;

                inline Enum(EEnumValue value) : value(value) {}
                operator EEnumValue() const { return (EEnumValue)value; }
            };

            inline Enum Read() { return Enum(Value_Read); }
            inline Enum Write() { return Enum(Value_Write); }
            inline Enum ReadWrite() { return Enum(Value_ReadWrite); }

        } // namespace EFileAccess

        namespace EFileOp
        {
            enum EEnumValue
            {
                Value_Sync,
                Value_Async,
            };

            struct Enum
            {
                s8 value;

                inline bool IsSync() const { return value == Value_Sync; }
                inline bool IsAsync() const { return value == Value_Async; }

                const char* ToString() const;

                inline Enum(EEnumValue value) : value(value) {}
                operator EEnumValue() const { return (EEnumValue)value; }
            };

            inline Enum Sync() { return Enum(Value_Sync); }
            inline Enum Async() { return Enum(Value_Async); }
        } // namespace EFileOp

        namespace EFileError
        {
            enum EEnumValue
            {
                ERROR_OK,
                ERROR_NO_FILE,         ///< File does not exist
                ERROR_BAD_FD,          ///< File descriptor is invalid
                ERROR_PRIORITY,        ///< Specified priority is invalid
                ERROR_MAX_FILES,       ///< Exceeded the maximum number of files that can be handled simultaneously
                ERROR_MAX_ASYNC,       ///< Exceeded the maximum number of async operations that can be scheduled
                ERROR_DEVICE,          ///< Specified device does not exist
                ERROR_DEVICE_READONLY, ///< Specified device does not exist
                ERROR_UNSUPPORTED,     ///< Unsupported function for this device
                ERROR_CANNOT_MOUNT,    ///< Could not be mounted
                ERROR_ASYNC_BUSY,      ///< Asynchronous operation has not completed
                ERROR_NOASYNC,         ///< No asynchronous operation has been performed
                ERROR_NOCWD,           ///< Current directory does not exist
                ERROR_NAMETOOLONG,     ///< Filename is too long
            };

            struct Enum
            {
                s8 value;

                inline bool IsOk() { return value == ERROR_OK; }
                inline bool IsNoFile() { return value == ERROR_NO_FILE; }
                inline bool IsBadf() { return value == ERROR_BAD_FD; }
                inline bool IsPriority() { return value == ERROR_PRIORITY; }
                inline bool IsMaxFiles() { return value == ERROR_MAX_FILES; }
                inline bool IsMaxAsync() { return value == ERROR_MAX_ASYNC; }
                inline bool IsDevice() { return value == ERROR_DEVICE; }
                inline bool IsDeviceReadonly() { return value == ERROR_DEVICE_READONLY; }
                inline bool IsUnsupported() { return value == ERROR_UNSUPPORTED; }
                inline bool IsCannotMount() { return value == ERROR_CANNOT_MOUNT; }
                inline bool IsAsyncBusy() { return value == ERROR_ASYNC_BUSY; }
                inline bool IsNoAsync() { return value == ERROR_NOASYNC; }
                inline bool IsNoCwd() { return value == ERROR_NOCWD; }
                inline bool IsNameTooLong() { return value == ERROR_NAMETOOLONG; }

                const char* ToString() const;

                inline Enum(EEnumValue value) : value(value) {}
                operator EEnumValue() const { return (EEnumValue)value; }
            };

            inline Enum Error_Ok() { return Enum(ERROR_OK); }
            inline Enum Error_NoFile() { return Enum(ERROR_NO_FILE); }
            inline Enum Error_BadFileDescriptor() { return Enum(ERROR_BAD_FD); }
            inline Enum Error_Priority() { return Enum(ERROR_PRIORITY); }
            inline Enum Error_MaxFiles() { return Enum(ERROR_MAX_FILES); }
            inline Enum Error_MaxAsync() { return Enum(ERROR_MAX_ASYNC); }
            inline Enum Error_Device() { return Enum(ERROR_DEVICE); }
            inline Enum Error_DeviceReadonly() { return Enum(ERROR_DEVICE_READONLY); }
            inline Enum Error_Unsupported() { return Enum(ERROR_UNSUPPORTED); }
            inline Enum Error_CannotMount() { return Enum(ERROR_CANNOT_MOUNT); }
            inline Enum Error_AsyncBusy() { return Enum(ERROR_ASYNC_BUSY); }
            inline Enum Error_NoAsync() { return Enum(ERROR_NOASYNC); }
            inline Enum Error_NoCwd() { return Enum(ERROR_NOCWD); }
            inline Enum Error_NameTooLong() { return Enum(ERROR_NAMETOOLONG); }

        } // namespace EFileError

    } // namespace nfs
}; // namespace ncore

#endif // __C_FILESYSTEM_ENUMERATIONS_H__
