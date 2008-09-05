#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/sock.h>
#include <skates/console.h>
#include <skates/rlist.h>
#include <skates/fdwatch.h>

static int default_func(CONSOLE_CONNECTION* conn, const char* name, const char* line);

int main(int argc, char* argv[])
{
	SOCK_ADDR sa;
	CONSOLE_INSTANCE* instance;

	sock_init();
	fdwatch_init();

	sock_str2addr("0.0.0.0:1980", &sa);
	instance = console_create(&sa, 100, 10);

	console_hook(instance, "echo", default_func);
	console_hook(instance, "setvalue", default_func);
	console_hook(instance, "getvalue", default_func);

	getchar();
	console_destroy(instance);

	fdwatch_final();
	sock_final();

	return 0;
}

int default_func(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	if(strcmp(name, "echo")==0) {
		return console_puts(conn, ERR_NOERROR, line);
	}
	if(strcmp(name, "getvalue")==0) {
		return console_puts(conn, ERR_NOERROR, "100");
	}
	if(strcmp(name, "setvalue")==0) {
		return console_puts(conn, ERR_NOERROR, "100");
	}

	return ERR_UNKNOWN;
}
