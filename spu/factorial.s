start:
	in
	pop ax

	call fact

	print is
	out ex
	hlt



fact:
	mov bx, 1
	mov ex, 1

fact_loop:
	push ax		; Decrement
	push 1
	sub
	pop ax

	print looping

	push bx		; bx + 1
	push 1
	add

	push ex

	mul		; cx * counter 
	pop ex


	jg fact_loop

	ret

is:
	db "factorial: ", 0

looping:
	db "looping!", 10, 0
