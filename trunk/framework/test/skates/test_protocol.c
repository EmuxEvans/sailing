#include <stdio.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

static char buf[100*1024];

static int on_new_node_begin(void* ptr, const char* name);
static int on_new_node_end(void* ptr);
static int on_new_const(void* ptr, const char* type, const char* name, const char* value);
static int on_new_field_def(void* ptr, const char* mode, const char* type, const char* name, const char* value);
static int on_new_array_def(void* ptr, const char* mode, const char* type, const char* name, const char* count);
static int on_new_field(void* ptr, const char* name, const char* value);
static int on_new_array(void* ptr, const char* name);
static int on_new_begin(void* ptr);
static int on_new_item(void* ptr, const char* value);
static int on_new_end(void* ptr);

static PROTOCOL_CALLBACK callback = {
	on_new_node_begin,
	on_new_node_end,
	on_new_const,
	on_new_field_def,
	on_new_array_def,
	on_new_field,
	on_new_array,
	on_new_begin,
	on_new_item,
	on_new_end,
};

int main(int argc, char* argv[])
{
	int len;

	if(argc!=2) abort();
	len = loal_textfile(argv[1], buf, sizeof(buf));
	if(len<=0) {
		printf("error in load file %s\n", argv[1]);
		return(0);
	}

	protocol_parse(buf, &callback, NULL);

	return(0);
}

int on_new_node_begin(void* ptr, const char* name)
{
	return 0;
}

int on_new_node_end(void* ptr)
{
	return 0;
}

int on_new_const(void* ptr, const char* type, const char* name, const char* value)
{
	return 0;
}

int on_new_field_def(void* ptr, const char* mode, const char* type, const char* name, const char* value)
{
	return 0;
}

int on_new_array_def(void* ptr, const char* mode, const char* type, const char* name, const char* count)
{
	return 0;
}

int on_new_field(void* ptr, const char* name, const char* value)
{
	return 0;
}

int on_new_array(void* ptr, const char* name)
{
	return 0;
}

int on_new_begin(void* ptr)
{
	return 0;
}

int on_new_item(void* ptr, const char* value)
{
	return 0;
}

int on_new_end(void* ptr)
{
	return 0;
}
