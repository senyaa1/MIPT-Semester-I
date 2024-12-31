start:
	in ax
	pop ax

	call fact

	print is
	out ex
	hlt



fact:
	mov bx, 0
	mov ex, 1

fact_loop:
	push ax		; dec
	push 1
	sub
	pop ax

	push bx		; bx + 1
	push 1
	add
	pop bx

	push bx		; bx * ex
	push ex
	mul
	pop ex

	# out ex


	cmp ax, 1
	jg fact_loop

	ret

is:
	db "factorial: ", 0
