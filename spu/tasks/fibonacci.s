start:
	in 
	call fib
	out 
	hlt

fib:	
	pop ax
	push ax
	cmp ax, 1
	jle return

	push 1
	sub

	push ax
	push 2
	sub

	call fib

	pop bx
	pop cx
	push bx
	push cx

	call fib

	add

return:
	ret

