inline void				xfilepath::clear()										{ mBuffer[0] = '\0'; mLength = 0; }

inline s32				xfilepath::length() const								{ return mLength; }
inline s32				xfilepath::maxLength() const							{ return mBufferSize - 1; }
inline xbool			xfilepath::empty() const								{ return mLength == 0; }

inline char&			xfilepath::operator [] (s32 index)						{ ASSERT(index>=0 && index<mBufferSize); return mBuffer[index]; }
inline char				xfilepath::operator [] (s32 index) const				{ ASSERT(index>=0 && index<mBufferSize); return mBuffer[index]; }

inline const char*		xfilepath::c_str() const								{ return mBuffer; }

