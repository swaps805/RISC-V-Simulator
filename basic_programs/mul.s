.text
    addi x5, x0, 3        # x5 = 3 (multiplier)
    addi x6, x0, 8        # x6 = 8 (multiplicand)
    addi x10, x0, 0       # x10 = 0 (accumulator)
    addi x11, x0, 0       # x11 = 0 (loop counter)

    # Jump to loop
    jal x1, loop

loop:
    bge x11, x6, Exit     # If x11 >= x6, go to Exit
    add x10, x10, x5      # x10 += x5 (accumulate)
    addi x11, x11, 1      # Increment loop counter
    jal x0, loop          # Jump back to the start of the loop

Exit:
    addi x10, x10, 0      # End program (do nothing)