lui x2, 0x50
addi x10, x0, 5
addi x11, x0, 6

addi x5, x0, 0xabcdef
addi x6, x0, 48

jal x1 leaf_example
beq x0, x0, end

leaf_example:
  addi, sp, sp, -16
  sd x5, 0(sp)
  sd x6, 8(sp)
  add x5, x10, x11
  addi x10, x5, 0
  ld x5, 0(sp)
  addi sp, sp, 8
  jalr x0, 0(x1) 

end:
	add x0, x0, x0