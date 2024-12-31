start:
	fin ax
	fin bx
	fin cx

	cmp ax, 0
	jz linear_jump

	call quadratic	
	jmp exit

linear_jump:
	call linear

exit:
	hlt


linear:
	push bx
	push 0
	fcmp 
	jz check_c		; if b == 0, 

	print roots_1

	push cx
	push bx
	fdiv

	print str_root_is
	fout 

	jmp linear_done

	check_c:
		fcmp cx, 0		
		jz linear_inf_roots

		print roots_0		; no roots
		jmp linear_done
		linear_inf_roots:	; inf roots
			print roots_inf
			jmp linear_done


	linear_done:
		ret




quadratic:
	push bx		; b ^ 2
	push bx
	fmul

	# fout 
	push ax		; a * c
	push cx
	fmul
	pop dx
	push dx
	push dx
	fadd
	push dx
	push dx
	fadd
	fadd
	fsub

	pop dx
	push dx
	# fout dx

	push 0
	fcmp 
	jl quadratic_no_roots
	jz quadratic_one_root

	push ax
	push ax
	fadd
	pop ax

	push dx
	sqrt	
	pop dx


	push dx
	push bx
	fsub
	push ax
	fdiv
	pop ex

	push 0
	push bx
	fsub
	push dx
	fsub

	push ax
	fdiv
	pop dx

	print roots_2
	fout dx
	fout ex 
	jmp quadratic_done

	quadratic_one_root:
		print roots_1
		fout dx
		jmp quadratic_done


	quadratic_no_roots:
		print roots_0
		jmp quadratic_done


	quadratic_done:
		ret


roots_0:
	db "no roots!", 10, 0

roots_1:
	db "1 root!", 10, 0

roots_2:
	db "2 roots!", 10, 0

roots_inf:
	db "infinite roots!", 10, 0

str_root_is:
	db "root is: ", 0

str_roots_are:
	db "roots are: ", 0
