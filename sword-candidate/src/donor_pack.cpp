#include <cstdint>
#include <cstring>
#define FRAME_API extern "C" __declspec(dllexport)
namespace {
uint32_t narrowNearest(double value) {
    uint64_t bits;
    memcpy(&bits,&value,sizeof(bits));
    auto sign=uint32_t((bits>>32)&0x80000000u);
    auto exponent=unsigned((bits>>52)&0x7ffu);
    auto mantissa=bits&0xfffffffffffffull;
    if (exponent==0x7ff) return sign|0x7f800000u|(mantissa?0x400000u|uint32_t(mantissa>>29):0);
    int e=int(exponent)-1023;
    if (e<-150) return sign;
    if (e>127) return sign|0x7f800000u;
    const unsigned shift=e>=-126?29u:unsigned(-e-97);
    const uint64_t full=mantissa|0x10000000000000ull, half=1ull<<(shift-1);
    const uint64_t tail=full&((half<<1)-1);
    auto rounded=uint32_t(full>>shift);
    rounded+=tail>half||(tail==half&&(rounded&1));
    if (e<-126) return sign|rounded;
    if (rounded==0x1000000u) { ++e; rounded>>=1; }
    if (e>127) return sign|0x7f800000u;
    return sign|(uint32_t(e+127)<<23)|(rounded&0x7fffffu);
}
}

FRAME_API int __stdcall FramePackDonor(const double* data, uint32_t* output) {
    constexpr size_t count=1296/4;
    const auto* command=reinterpret_cast<const uint8_t*>(data+count);
    for (size_t i=0;i<count;++i) {
        if (command[i]==1) output[i]=narrowNearest(data[i]);
        else if (command[i]==2) output[i]=static_cast<uint32_t>(data[i]);
    }
    return 1;
}
