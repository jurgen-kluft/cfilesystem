# __xfilesystem__

## __Cross platform filesystem library__

* xfilesystem
  * filepath
  * dirpath
  * file info
  * dir info
  * file
  * stream

## __TODO__

* Implement file device for Win32 and Mac
* Have xfilepath and xdirpath use xpath
* Debug filesystem init for Win32 and Mac (use wchar API)
* Implement file stream and memory stream
* Finish device manager (use xpath)

## __API__

```c++
// API
struct xfile;
struct xinfo;

class xfilesystem
{
public:
    enum emode
    {
        READ       = 0x1,
        WRITE      = 0x2,
        OPEN       = 0x10 | READ,
        CREATE     = 0x20 | WRITE,
        DISCARD    = 0x40 | WRITE,
        APPEND     = 0x80 | WRITE,
        INVALID    = 0xffffffff,
    };

    xfile*      open(xfilepath const& filename, emode mode);
    xwriter*    writer(xfile*);
    xreader*    reader(xfile*);

    void        close(xfile*);
    void        close(xinfo*);
    void        close(xreader*);
    void        close(xwriter*);

    s64         get_pos(xfile*);
    s64         set_pos(xfile*, s64 pos);

    xinfo*      info(xfilepath const& path);
    bool        exists(xinfo*);
    s64         size(xinfo*);
    xfile*      open(xinfo*, emode mode);
    void        rename(xinfo*, xfilepath const&);
    void        move(xinfo* src, xinfo* dst);
    void        copy(xinfo* src, xinfo* dst);
    void        remove(xinfo*);

    s32         read(reader*, xbuffer&);
    s32         write(writer*, xcbuffer const&);

    void        read_async(reader*, xbuffer&);
    s32         wait_async(reader*);
};
```

// ==================================================================================
// USE CASE: Initialization of a file-system

```c++
xfilesyscfg cfg;
cfg.slash = '\\';                           // The filesystem slash of the system
cfg.allocator = xalloc::get_system();       // You can also give it its own allocator
xfilesystem* xfs = xfilesystem::create(config)
```

// ======================================================================  
// USE CASE: Opening reading/writing files  

```c++
// NOTE: You can use forward and/or backwards slashes, they will be corrected  
//       automatically  
// NOTE: Reader and Writer API does not expose seek functionality, if you need  
//       this you should create a stream.

// SYNC

xfile* f = xfs->open("d:/test.bin", xfilesystem::READ);
xreader* reader = xfs->reader(f);
s32 numbytesread = xfs->read(reader, xbuffer& data);
xfs->close(reader);
xfs->close(f);

xfile* f = xfs->open("d:/test.bin", xfilesystem::READ_WRITE);
xwriter* writer = xfs->writer(f);
s32 numbyteswritten = xfs->write(writer, xcbuffer& data);
xfs->close(writer);
xfs->close(f);

// ASYNC

xfile* f = xfs->open("d:/test.bin", xfilesystem::READ);
xreader* reader = xfs->reader(f);
xfs->read_async(reader, xbuffer& data);
xfs->wait_async(reader);
xfs->close(reader);
xfs->close(f);
```

// =====================================================================  
// USE CASE: Attributes and times of a file

```c++
xfileinfo finfo = xfs->info("E:/Dev.c++/main.cpp");
xfileattrs fattrs = finfo->getAttrs();
xfiletimes ftimes = finfo->getTimes();

fattrs.setArchive(true);
finfo->setAttrs(fattrs);
```

// ===================================================================  
// USE CASE: Globbing, navigating and searching files

```c++
xdirpath dir = mfs->dirpath("E:/data");
myfs->search(dir, [](xfileinfo const* fileinfo, xdirinfo const* dirinfo, s32 dirdepth){
    if (dirinfo!=nullptr)
    {
        // Do not recurse into any directory
        return false;
    } else if (fileinfo!=nullptr) {
        if (fileinfo->getFilepath() == "README.md")
        {
            // Do something with it and end the search
            return false;
        }
    }
    return true;    // Continue the search
});
```

// =====================================================================  
// USE CASE: Text based reading  

```c++
// NOTE: This requires you to add a dependency on xtext

xfile* f = xfs->open("d:\\test.bin", xfilesystem::READ);
xreader* reader = xfs->reader(f);
//NOTE: xtextreader in the xtext package ?
xtextreader* textreader = xnew<xtextreader>(reader);
//xtextreader* textreader = xfs->xnew<xtextreader>(reader); // When you want to use xfilesystem memory

xstring line;
while (textreader.readLine(line))
{
    // Do something with 'line'
}

xtextreader->close();
xdelete<>(textreader);

xfs->close(reader);
xfs->close(f);
```
