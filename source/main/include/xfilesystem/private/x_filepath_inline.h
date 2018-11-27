

namespace xcore
{
    void        fixSlashes(xstring& str, uchar32 old_slash, uchar32 new_slash);
    void		setOrReplaceDeviceName(xstring& ioStr, xstring const& inDeviceName) const;
    void		setOrReplaceDevicePart(xstring& ioStr, xstring const& inDeviceName) const;

}