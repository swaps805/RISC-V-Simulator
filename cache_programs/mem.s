.data
.dword 0xa55aa5a593933939
.dword 0x39933939a55aa5a5
.text
    lui x1, 0x10
main:
    lhu x3, 0(x1)
    lh x4, 0(x1)
    lh x5, 2(x1)
    ld x6, 0(x1)
    lw x7, 12(x1)
    lbu x8, 7(x1)
    lb x9, 7(x1)
    lb x10, 6(x1)
    ld x11, 3(x1)
    lwu x12, 6(x1)
