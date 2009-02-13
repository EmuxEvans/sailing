#include <v8.h>

static v8::Handle<v8::Value> Print(const v8::Arguments& args)
{
	return v8::Undefined();
}

int main(int argc, char* argv[])
{
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
	v8::Handle<v8::ObjectTemplate> global_templ = v8::ObjectTemplate::New();

	global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	global_templ->Set(v8::String::New("callme"), v8::PropertyAttribute

	return 0;
}
