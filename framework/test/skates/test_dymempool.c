#include <stdio.h>
#include <stdlib.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/dymempool.h>

static void test_proc();

int main(int argc, char* argv[])
{
	dymempool_init(300, 64*1024);

	test_proc();

	dymempool_final();
	return(0);
}

void test_proc()
{
	void *v1;
	v1 = dymempool_alloc(100);
	v1 = dymempool_realloc(150, v1);
	v1 = dymempool_realloc(9000, v1);
	dymempool_free(v1);
}

