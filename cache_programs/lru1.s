.text

lui x3, 0x10
addi, x4, x0, 10
sd x4, 0(x3)
addi x4, x0, 20
sd x4, 16(x3)
addi x4, x0, 30
sd x4, 32(x3)
addi x4, x0, 40
sd x4, 48(x3)
addi x4, x0, 50
sd x4, 64(x3)

ld x5, 0(x3)
ld x5, 16(x3)
ld x5, 32(x3)
ld x5, 48(x3)
addi x5, x0, 72
sd x5, 16(x3)
ld x5, 16(x3)
ld x5, 64(x3)
ld x5, 32(x3)
ld x5, 0(x3)