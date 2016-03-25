#ifndef EBRANCH_H
#define EBRANCH_H

#include <string>

enum class EBranchT
{
    T1, //user search
    T2, //context
    T3, //category
    T4, //short
    T5, //long
    TMAX
};

enum class EBranchL
{
    L0, L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12,
    L13, L14, L15, L16, L17, L18, L19, L20, L21, L22, L23,
    L24, L25, L26, L27, L28, L29, L30, L31, L32, L33, L34, L35, L36, L37, LMAX
};

extern const std::string EBranchL2String(EBranchL);

#endif // EBRANCH_H
