.data
.dword 3, 67, 128, 0, 100, 32, 1280
.text

lui x3, 0x10 # base adress
ld x15, 0(x3)
add, x15, x15, x15 # 2*n n = number of elements
addi, x2, x0, 0 # i pointer 
addi, x14, x0, 0 # j pointer (to store gcd in final location)
lui x12, 0x10
addi x12, x12, 0x50

loop:
    bge x2, x15, exit # i < 2*n
    addi x2, x2, 1 # i += 1
    slli x16, x2, 3 # offset
    add x4, x3, x16
    ld x7, 0(x4) # load a in x7
    
    addi x2, x2, 1 # i += 1
    slli x16, x2, 3
    add x4, x3, x16
    ld x8, 0(x4) # load b in x8
    
    beq x7, x0, exit_gcd # if either a or b is 0 return 0
    beq x8, x0, exit_gcd
    blt x7, x8, swap # ensure a > b else swap

# gcd calculation
gcd:
    beq x0, x8, exit_gcd # if b == 0, a is gcd
    add, x5, x0, x7 # strore a in x5
    add, x6, x0, x8 # store b in x6
sub_loop: # caluclate a % b
    blt x5, x6, update
    sub, x5, x5, x6 # rem in x5
    beq x0, x0, sub_loop
update:
    add x7, x0, x6 # a = b
    add x8, x0, x5 # b = a%b
    beq x0, x0, gcd
swap:
    add x18, x0, x7 # t = a
    add x7, x0, x8  # a = b
    add x8, x0, x18 # b = t
    beq x0, x0, gcd
    
exit_gcd:
    slli x16, x14, 3 # offset into memory
    add x11, x12, x16
    sd x7, 0(x11)
    addi x14, x14, 1 # j += 1
    beq x0, x0, loop
exit:
    add x0, x0, x0
