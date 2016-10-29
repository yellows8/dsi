#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h>
#include <stdarg.h>
#include "vsprintf.h"
#include <nds.h>

//const devoptab_t *devoptab_list[];

//int fifoInit(){}

static char print_strbuf[0x200];
int printf(const char *format, ...)
{
	int ret = 0, i;
	va_list args;

	#ifdef ARM9
	memset(print_strbuf, 0, 0x200);
	va_start(args, format);
	ret = vsnprintf(print_strbuf, 0x1ff, format, args);
	REG_IME = 0;
	for(i=0; i<strlen(print_strbuf); i++)consolePrintChar(print_strbuf[i]);
	REG_IME = 1;
	#endif

	return ret;
}

int iprintf(const char *format, ...)
{
	int ret = 0, i;
	va_list args;

	#ifdef ARM9
	memset(print_strbuf, 0, 0x200);
	va_start(args, format);
	ret = vsnprintf(print_strbuf, 0x1ff, format, args);
	for(i=0; i<strlen(print_strbuf); i++)consolePrintChar(print_strbuf[i]);
	#endif

	return ret;
}

int sprintf(char *str, const char *format, ...)
{
	int ret = 0, i;
	va_list args;

	va_start(args, format);
	ret = vsprintf(str, format, args);

	return ret;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
	int ret = 0, i;
	va_list args;

	va_start(args, format);
	ret = vsnprintf(str, size, format, args);

	return ret;
}

int sscanf(const char *str, const char *format, ...)
{
	return 0;
}

int siscanf(const char *str, const char *format, ...)
{
	return 0;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
	return 0;
}

int puts(const char *s)
{
	#ifdef ARM9
	int i;
	for(i=0; i<strlen(s); i++)consolePrintChar(s[i]);
	#endif

	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	unsigned char *a, *b;
	a = (unsigned char*)dest;
	b = (unsigned char*)src;
	for(; n>0; n--)*a++ = *b++;
	return dest;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *a;
	a = (unsigned char*)s;
	for(; n>0; n--)*a++ = (unsigned char)c;
	return s;
}

char *strcpy(char *dest, const char *src)
{
	char *a, *b, c;
	a = dest;
	b = (char*)src;

	while(1)
	{
		c = *b++;
		*a++ = c;
		if(c==0)break;
	}

	return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *a, *b, c;
	a = dest;
	b = (char*)src;

	while(1)
	{
		c = *b++;
		*a++ = c;
		n--;
		if(c==0 || n<=0)break;
	}

	while(n>0)
	{
		*a++ = 0;
		n--;
	}

	return dest;
}

