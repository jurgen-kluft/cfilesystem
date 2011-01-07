
extern void x_StdioInit();
extern void x_StdioExit();

//==============================================================================
// XFILE 
//==============================================================================

//------------------------------------------------------------------------------
// Author:
//     Tomas Arce
// Summary:
//     Write a object to file.
// Arguments:
//     val    - The object to be written. The function only care the address and size of template T.
// Returns:
//     void
// Description:
//.....The function write the data as chars from val address starts to file. The size of data output is the size of T. The data will be written at position specified by current file pointer.
// See Also:
//     xfile::write(T* val, s32 count)
//------------------------------------------------------------------------------
template<class T> inline
void xfile::write(T& val)
{
	writeRaw(&val, sizeof(T), 1);
}

//------------------------------------------------------------------------------
// Author:
//     Tomas Arce
// Summary:
//     Write a object to file in times.
// Arguments:
//     val    - The pointer of object to be written. The function only care the size of template T.
//     count  - The times val will be written.
// Returns:
//     void
// Description:
//.....The function write the data as chars from address val point to file. The total size of data output is the sizeof(T)*count. The data will be written at position specified by current file pointer.
// See Also:
//     xfile::write(T& val)
//------------------------------------------------------------------------------
template<class T> inline
void xfile::write(T* val, s32 count)
{
	writeRaw(val, sizeof(T), count);
}

//------------------------------------------------------------------------------
// Author:
//     Tomas Arce
// Summary:
//     Read data from file to the specified object.
// Arguments:
//     val    - The data will be written to the object val.
// Returns:
//     Returns xTRUE, if read success. Return xFALSE, if read fails(Only reach end of the file).
// Description:
//.....The function read the data as chars from file to object val. The total size of data read is the sizeof(T). The data will be read from the position the current file pointer indicates.
// See Also:
//     xfile::read(T* val, s32 count)
//------------------------------------------------------------------------------
template<class T> inline
xbool xfile::read(T& val)
{
	xbool isSuccess;
	isSuccess = readRaw(&val, sizeof(T), 1);
	return isSuccess;
}

//------------------------------------------------------------------------------
// Author:
//     Tomas Arce
// Summary:
//     Read data from file to the specified object in times.
// Arguments:
//     val    - The data will be written to the object val.
//     count    - The times that the data to be written to the object val.
// Returns:
//     Returns xTRUE, if read success. Return xFALSE, if read fails(Only reach end of the file).
// Description:
//.....The function read the data as chars from file to object val. It reads from file "count" times. If reached end of the file, the process break and return xFALSE.
// See Also:
//     xfile::read(T* val)
//------------------------------------------------------------------------------
template<class T> inline
xbool xfile::read(T* val, s32 count)
{
	xbool isSuccess;
	isSuccess = readRaw(val, sizeof(T), count);
	return isSuccess;
}