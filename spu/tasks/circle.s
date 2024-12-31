start:
	mov ax, 0

	loop:
		push fb_start
		push ax
		add
		pop bx

		push ax
		push [fb_width]
		div
		pop cx

		push ax
		push [fb_width]
		mod
		pop dx

		# out bx
		# out cx
		# out dx

		push cx
		push 180
		sub
		pop cx

		push dx
		push 240
		sub
		pop dx

		push cx
		push cx
		mul
		push dx
		push dx
		mul
		add

		push [radius]

		cmp
		jle draw_white

		mov [bx], 0x00
		jmp continue

		draw_white:
			mov [bx], 0xff
			jmp continue

		continue:
			push ax
			push 1
			add
			pop ax
			cmp ax, [fb_size]
			jle loop

	draw fb_start
	sleep 5
	hlt

radius:
	dd 10000

fb_size:
	dd 172800

fb_width:
	dd 480

fb_height:
	dd 360

fb_start:
