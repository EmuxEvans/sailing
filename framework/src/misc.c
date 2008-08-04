#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/misc.h"

void strtrim(char* str)
{
	size_t s;
	for(s=strlen(str); s>0; s--) {
		if(str[s-1]!=' ') break;
	}
	str[s] = '\0';
}

void strltrim(char* str)
{
	char* cur;
	for(cur=str; *cur!='\0'; cur++) {
		if(*cur!=' ') {
			if(cur!=str) memmove(str, cur, strlen(cur)+1);
			return;
		}
	}
}

static unsigned char HEXVAL[] =				// used for convertion from string to binary
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static char HEXCHAR[] = "0123456789ABCDEF";

int hex2bin(const char* in, unsigned char* out, int outlen)
{
	int i;

	if(outlen==-1) {
		assert(strlen(in)%2==0);
		outlen = (int)strlen(in)/2;
	}

	for(i=0; i<outlen; i++) {
		if(HEXVAL[(unsigned int)in[i*2+0]]==0xFF) return 0;
		if(HEXVAL[(unsigned int)in[i*2+1]]==0xFF) return 0;

		out[i] = (HEXVAL[(unsigned int)in[i*2]]<<4) | (HEXVAL[(unsigned int)in[i*2+1]]);
	}

	return 1;
}

void bin2hex(const unsigned char* in, int buflen, char* out)
{
	int i;
	for(i=0; i<buflen; i++) {
		out[i*2] 	= HEXCHAR[(int)(in[i]>>4)];
		out[i*2+1] 	= HEXCHAR[(int)(in[i]&0x0F)];
	}
	out[i*2] = '\0';
}

const char* strget(const char* str, char split, char* out, size_t out_size)
{
	const char* end;
	size_t len;

	end = strchr(str, split);
	if(end==NULL) {
		len = strlen(str);
		if(len>=out_size) return NULL;
		strcpy(out, str);
		return str+len;
	} else {
		len = end - str;
		if(len>=out_size) return NULL;
		memcpy(out, str, len);
		out[len] = '\0';
		return end+1;
	}
}

const char* strget2int(const char* str, char split, int* value)
{
	char sval[100];
	str = strget(str, split, sval, sizeof(sval));
	if(str==NULL) return NULL;
	*value = atoi(sval);
	return str;
}

const char* strget2float(const char* str, char split, float* value)
{
	char sval[100];
	str = strget(str, split, sval, sizeof(sval));
	if(str==NULL) return NULL;
	*value = (float)atof(sval);
	return str;
}

char* strput(char* str, size_t len, char split, const char* in)
{
	size_t in_len;
	size_t str_len;
	in_len = strlen(in);
	str_len = strlen(str);
	if(str_len+in_len+1>len) return NULL;
	if(str_len!=0) {
		str[str_len] = split;
		str_len += 1;
	}
	memcpy(str+str_len, in, in_len);
	str[str_len+in_len] = '\0';
	return str;
}

char* strput4int(char* str, size_t len, char split, int value)
{
	char sval[100];
	sprintf(sval, "%d", value);
	return strput(str, len, split, sval);
}

char* strput4float(char* str, size_t len, char split, float value)
{
	char sval[100];
	sprintf(sval, "%f", value);
	return strput(str, len, split, sval);
}

const char* strget_space(const char* str, char* out, size_t out_size)
{
	const char* end;
	size_t len;

	while(*str==' ') {
		str++;
		if(*str=='\0') return NULL;
	}

	end = strchr(str, ' ');
	if(end==NULL) {
		len = strlen(str);
		if(len>=out_size) return NULL;
		strcpy(out, str);
		return str+len;
	} else {
		len = end - str;
		if(len>=out_size) return NULL;
		memcpy(out, str, len);
		out[len] = '\0';
		while(*end==' ') end++;
		return end;
	}
}

const char* strget2int_space(const char* str, int* out)
{
	char sval[100];
	str = strget_space(str, sval, sizeof(sval));
	if(str==NULL) return NULL;
	*out = atoi(sval);
	return str;
}

const char* strget2float_space(const char* str, float* out)
{
	char sval[100];
	str = strget_space(str, sval, sizeof(sval));
	if(str==NULL) return NULL;
	*out = (float)atof(sval);
	return str;
}

char* strput_space(char* str, size_t len, const char* in)
{
	return strput(str, len, ' ', in);
}

char* strput4int_space(char* str, size_t len, int value)
{
	char sval[100];
	sprintf(sval, "%d", value);
	return strput_space(str, len, sval);
}

char* strput4float_space(char* str, size_t len, float value)
{
	char sval[100];
	sprintf(sval, "%f", value);
	return strput_space(str, len, sval);
}

int loal_textfile(const char* filename, char* buf, int buflen)
{
	FILE* fp;
	int len = 0;

	fp = fopen(filename, "rt");
	if(fp==NULL) return 0;

	for(;;) {
		if(fgets(buf+len, buflen-len, fp)==NULL) {
			break;
		}
		len += (int)strlen(buf+len);
	}

	fclose(fp);
	return len+1;
}

int save_textfile(const char* filename, char* buf, int buflen)
{
	return 0;
}
