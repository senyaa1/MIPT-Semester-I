#define __USE_GNU

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <ucontext.h>

#include "io.h"

static struct sigaction siga;

void print_buf(uint8_t* buf, size_t sz)
{
	printf("\n");
	for(int i = 0; i < sz; i++)
		printf("%02hhX ", buf[i]);
	printf("\n");
}


int framebuffer_initialize(framebuf_t* fb)
{
	int fbfd = open(FB_DEV, O_RDWR);

	if (fbfd == -1) return -1;

	ioctl(fbfd, FBIOGET_VSCREENINFO, &fb->vinfo);
	ioctl(fbfd, FBIOGET_FSCREENINFO, &fb->finfo);

	fb->addr = mmap(NULL, fb->vinfo.xres * fb->vinfo.yres * fb->vinfo.bits_per_pixel / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

	printf("width: %d\theight: %d\n", fb->vinfo.xres, fb->vinfo.yres);

	if(fb->addr == MAP_FAILED) return -1;

	return 0;
}

void framebuffer_destroy(framebuf_t* fb)
{
	if(fb->addr == 0)  return;
	munmap(fb->addr, FB_SIZE);
	close(fb->fd);
}

void framebuffer_draw(framebuf_t* fb, uint8_t* dataptr)
{
	size_t y_offset = (fb->vinfo.yres - FB_HEIGHT) / 2;
	size_t x_offset = (fb->vinfo.xres - FB_WIDTH) / 2;
	for (size_t y = 0; y < FB_HEIGHT; y++)
	{
		for (size_t x = 0; x < FB_WIDTH; x++) 
		{
			size_t location = (x + fb->vinfo.xoffset + x_offset) * (fb->vinfo.bits_per_pixel/8) + (y + fb->vinfo.yoffset + y_offset) * fb->finfo.line_length;

			int r = dataptr[y * FB_WIDTH + x];
			int g = dataptr[y * FB_WIDTH + x];
			int b = dataptr[y * FB_WIDTH + x];

			if (fb->vinfo.bits_per_pixel == 32) 
			{
				*(fb->addr + location) = b;
				*(fb->addr + location + 1) = g;
				*(fb->addr + location + 2) = r;
				*(fb->addr + location + 3) = 0;
			} 
			else  
			{ 
				*((unsigned short int*)(fb->addr + location)) = (r << 11 | g << 5 | b);
			}

		}
	}
}

void setup_signals(void(*signal_handler)(int sig, siginfo_t* si, void* arg))
{
	siga.sa_sigaction = signal_handler;
	siga.sa_flags = SA_SIGINFO;

	for(size_t i = 0; i < sizeof(subscribed_signals) / sizeof(int); i++)
	{
		if(sigaction(subscribed_signals[i], &siga, NULL))
		{
			perror("sigaction");
			exit(EXIT_FAILURE);
		}
	}
}
