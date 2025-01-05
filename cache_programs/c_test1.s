.data
.dword 0xabcdef0123456789
.dword 0xfdfdfdfdfdfdfdfd
.dword 0xabababababababab

.text
lui x3 0x10
ld x7, 1(x3)
ld x9, 2(x3)
addi x3, x3, 0x81
ld x8, 0(x3)