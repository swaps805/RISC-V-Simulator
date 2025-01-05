.data
.dword 0x10, -10, 20, -20, 30, -30, 40, -40, 50, -50, 72, -72

.text
lui x3, 0x10
ld x5, 0(x3)
ld x5, 16(x3)
ld x5, 32(x3)
ld x5, 48(x3)
ld x5, 16(x3)
ld x5, 64(x3)
ld x5, 32(x3)
ld x5, 0(x3)