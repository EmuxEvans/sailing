#include <stdio.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

#include "sample_p.proto.h"

char buf[100000];
b vv;

static void my_new_field(void* ptr, const char* name, const char* value);
static void my_new_array(void* ptr, const char* name);
static void my_new_begin(void* ptr);
static void my_new_item(void* ptr, const char* value);
static void my_new_end(void* ptr);

static char callback_type[100];
static char callback_name[100];
static char callback_value[100];
static PROTOCOL_CALLBACK callback = {
	my_new_field,
	my_new_array,
	my_new_begin,
	my_new_item,
	my_new_end,

	callback_type,
	callback_name,
	callback_value,
	sizeof(callback_type),
	sizeof(callback_name),
	sizeof(callback_value),

	0
};

int main(int argc, char* argv[])
{
	unsigned int len;

	len = sizeof(buf);
	memset(&vv, 0, sizeof(vv));
	memset(buf, 0xff, sizeof(buf));
	protocol_binary_write(&PROTOCOL_NAME(b), &vv, buf, &len);
	memset(&vv, 0xff, sizeof(vv));
	protocol_binary_read(&PROTOCOL_NAME(b), buf, &len, &vv);
	len = sizeof(buf);
	memset(buf, 0xff, sizeof(buf));
	protocol_text_write(&PROTOCOL_NAME(b), &vv, buf, &len);

	protocol_parse(buf, &callback, NULL);
	return 0;
}

void my_new_field(void* ptr, const char* name, const char* value)
{
	printf("%s %s %s\n", __FUNCTION__, name, value);
}

void my_new_array(void* ptr, const char* name)
{
	printf("%s %s\n", __FUNCTION__, name);
}

void my_new_begin(void* ptr)
{
	printf("%s\n", __FUNCTION__);
}

void my_new_item(void* ptr, const char* value)
{
	printf("%s %s\n", __FUNCTION__, value);
}

void my_new_end(void* ptr)
{
	printf("%s\n", __FUNCTION__);
}
