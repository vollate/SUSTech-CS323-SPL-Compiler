# SPL compiler generated assembly
.data
_prmpt: .asciiz "Enter an integer: "
_eol: .asciiz "\n"
.globl main
.text
j main

read:
li $v0, 4
la $a0, _prmpt
syscall
li $v0, 5
syscall
jr $ra

write:
li $v0, 1
syscall
li $v0, 4
la $a0, _eol
syscall
move $v0, $0
jr $ra

hanoi:
addi $sp, $sp, -120
sw $ra, 116($sp)
sw $fp, 112($sp)
move $fp, $sp
# new var v0, store in reg $t0
move $t0, $a3
# new var v1, store in reg $t1
move $t1, $a2
# new var v2, store in reg $t2
move $t2, $a1
# new var v3, store in reg $t3
move $t3, $a0
# new var v4, store in reg $t4
move $t4, $t0
# new var v5, store in reg $t5
li $t5, 1
sw $t0, 108($sp) # spill v0 from reg $t0
sw $t1, 104($sp) # spill v1 from reg $t1
sw $t2, 100($sp) # spill v2 from reg $t2
sw $t3, 96($sp) # spill v3 from reg $t3
sw $t4, 92($sp) # spill v4 from reg $t4
sw $t5, 88($sp) # spill v5 from reg $t5
lw $t0, 92($sp) # load v4 to reg $t0
lw $t1, 88($sp) # load v5 to reg $t1
beq $t0, $t1, label0
j label1
label0:
# new var v9, store in reg $t0
lw $t1, 104($sp) # load v1 to reg $t1
move $t0, $t1
# new var v10, store in reg $t2
li $t2, 10000
# new var v8, store in reg $t3
mul $t3, $t0, $t2
# new var v11, store in reg $t4
lw $t5, 96($sp) # load v3 to reg $t5
move $t4, $t5
# new var v7, store in reg $t6
add $t6, $t3, $t4
move $a0, $t6
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
sw $t0, 84($sp) # spill v9 from reg $t0
sw $t2, 80($sp) # spill v10 from reg $t2
sw $t3, 76($sp) # spill v8 from reg $t3
sw $t4, 72($sp) # spill v11 from reg $t4
sw $t6, 68($sp) # spill v7 from reg $t6
j label2
label1:
# new var v14, store in reg $t0
lw $t1, 108($sp) # load v0 to reg $t1
move $t0, $t1
# new var v15, store in reg $t2
li $t2, 1
# new var v13, store in reg $t3
sub $t3, $t0, $t2
# new var v16, store in reg $t4
lw $t5, 104($sp) # load v1 to reg $t5
move $t4, $t5
# new var v17, store in reg $t6
lw $t7, 96($sp) # load v3 to reg $t7
move $t6, $t7
# new var v18, store in reg $t8
lw $t9, 100($sp) # load v2 to reg $t9
move $t8, $t9
move $a0, $t8
move $a1, $t6
move $a2, $t4
move $a3, $t3
sw $t0, 64($sp) # spill v14 from reg $t0
sw $t2, 60($sp) # spill v15 from reg $t2
sw $t3, 56($sp) # spill v13 from reg $t3
sw $t4, 52($sp) # spill v16 from reg $t4
sw $t6, 48($sp) # spill v17 from reg $t6
sw $t8, 44($sp) # spill v18 from reg $t8
# new var v12, store in reg $t0
jal hanoi
move $t0, $v0
# new var v22, store in reg $t1
lw $t2, 104($sp) # load v1 to reg $t2
move $t1, $t2
# new var v23, store in reg $t3
li $t3, 10000
# new var v21, store in reg $t4
mul $t4, $t1, $t3
# new var v24, store in reg $t5
lw $t6, 96($sp) # load v3 to reg $t6
move $t5, $t6
# new var v20, store in reg $t7
add $t7, $t4, $t5
move $a0, $t7
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
# new var v27, store in reg $t8
lw $t9, 108($sp) # load v0 to reg $t9
move $t8, $t9
# new var v28, store in reg $t2
li $t2, 1
# new var v26, store in reg $t6
sub $t6, $t8, $t2
# new var v29, store in reg $t9
sw $t0, 40($sp) # spill v12 from reg $t0
lw $t0, 100($sp) # load v2 to reg $t0
move $t9, $t0
# new var v30, store in reg $t0
sw $t1, 36($sp) # spill v22 from reg $t1
lw $t1, 104($sp) # load v1 to reg $t1
move $t0, $t1
# new var v31, store in reg $t1
sw $t0, 0($sp) # spill v30 from reg $t0
lw $t0, 96($sp) # load v3 to reg $t0
move $t1, $t0
move $a0, $t1
lw $t0, 0($sp) # load v30 to reg $t0
move $a1, $t0
move $a2, $t9
move $a3, $t6
sw $t1, -4($sp) # spill v31 from reg $t1
sw $t2, 12($sp) # spill v28 from reg $t2
sw $t3, 32($sp) # spill v23 from reg $t3
sw $t4, 28($sp) # spill v21 from reg $t4
sw $t5, 24($sp) # spill v24 from reg $t5
sw $t6, 8($sp) # spill v26 from reg $t6
sw $t7, 20($sp) # spill v20 from reg $t7
sw $t8, 16($sp) # spill v27 from reg $t8
sw $t9, 4($sp) # spill v29 from reg $t9
# new var v25, store in reg $t0
jal hanoi
move $t0, $v0
label2:
# new var v32, store in reg $t1
li $t1, 0
move $sp, $fp
lw $ra, 116($sp)
lw $fp, 112($sp)
addi $sp, $sp, 120
move $v0, $t1
jr $ra

main:
addi $sp, $sp, -24
sw $ra, 20($sp)
sw $fp, 16($sp)
move $fp, $sp
# new var v34, store in reg $t2
li $t2, 3
# new var v33, store in reg $t3
move $t3, $t2
# new var v36, store in reg $t4
move $t4, $t3
# new var v37, store in reg $t5
li $t5, 1
# new var v38, store in reg $t6
li $t6, 2
# new var v39, store in reg $t7
li $t7, 3
move $a0, $t7
move $a1, $t6
move $a2, $t5
move $a3, $t4
sw $t0, 16($sp) # spill v25 from reg $t0
sw $t1, 12($sp) # spill v32 from reg $t1
sw $t2, 12($sp) # spill v34 from reg $t2
sw $t3, 8($sp) # spill v33 from reg $t3
sw $t4, 4($sp) # spill v36 from reg $t4
sw $t5, 0($sp) # spill v37 from reg $t5
sw $t6, -4($sp) # spill v38 from reg $t6
sw $t7, -8($sp) # spill v39 from reg $t7
# new var v35, store in reg $t0
jal hanoi
move $t0, $v0
# new var v40, store in reg $t1
li $t1, 0
move $sp, $fp
lw $ra, 20($sp)
lw $fp, 16($sp)
addi $sp, $sp, 24
move $v0, $t1
jr $ra
