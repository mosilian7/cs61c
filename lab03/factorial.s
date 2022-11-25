.globl factorial

.data
n: .word 12

.text
main:
    la t0, n
    lw a0, 0(t0)
    jal ra, factorial

    addi a1, a0, 0
    addi a0, x0, 1
    ecall # Print Result

    addi a1, x0, '\n'
    addi a0, x0, 11
    ecall # Print newline

    addi a0, x0, 10
    ecall # Exit

factorial:
    # YOUR CODE HERE
	bne a0, x0, else # if a0 == 0 { finish }
    addi a0, x0, 1
	jr ra
    
else:
	addi sp, sp, -8
    sw a0, 0(sp)
    sw ra, 4(sp)
    
    addi a0, a0, -1
    jal factorial
    addi t0, a0, 0
    
    lw a0, 0(sp)
    lw ra, 4(sp)
    mul a0, a0, t0
    addi sp, sp, 8
    jr ra
	
    
    