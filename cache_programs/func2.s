
# dual leaf function f1 calls f2

lui x2, 0x50
addi x10, x0, 100
addi x11, x0, 5
addi x12, x0, 5

jal x1, f1
beq x0, x0, end

f1:
    addi sp, sp, -48
    sd x1, 24(sp)
    sd x10, 16(sp)
    sd x11, 8(sp)
    sd x12, 0(sp)
    addi x10, x11, 0
    jal x1, f2
    addi x5, x10, 0
    ld x12, 0(sp)
    ld x11, 8(sp)
    ld x10, 16(sp)
    ld x1, 24(sp)
    addi sp, sp, 32
    sub x6, x10, x5
    add x10, x6, x12
    addi sp, sp, 16
    jalr x0, 0(x1)

f2:
    slli x6, x10, 3
    addi x10, x6, 5
    jalr x0, x1(0)

end:
    add x0, x0, x0