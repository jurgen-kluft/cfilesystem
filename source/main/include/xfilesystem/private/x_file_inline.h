template<class T> inline
void xfile::write(T& val)
{
	writeRaw(&val, sizeof(T), 1);
}
template<class T> inline
void xfile::write(T* val, s32 count)
{
	writeRaw(val, sizeof(T), count);
}
template<class T> inline
xbool xfile::read(T& val)
{
	xbool isSuccess;
	isSuccess = readRaw(&val, sizeof(T), 1);
	return isSuccess;
}
template<class T> inline
xbool xfile::read(T* val, s32 count)
{
	xbool isSuccess;
	isSuccess = readRaw(val, sizeof(T), count);
	return isSuccess;
}