#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "idl_codegen.h"
#include "idl_cc.h"

static FILE* fp_h = NULL;
static FILE* fp_clt = NULL;
static FILE* fp_svr = NULL;

static int gen_h(const char* filename);
static int gen_client(const char* filename);
static int gen_server(const char* filename);
static void gen_plist(char* str, PARAMETER p_list[], int p_count);
static void gen_proxy(FUNCTION* func);
static void gen_stub(FUNCTION* func);

//#define OUTPUT_DEBUG

int gencode(const char* path, const char* filename)
{
	char fname[200];
	
	sprintf(fname, "%s/%s.h", path, filename);
	fp_h = fopen(fname, "w");
	if(fp_h==NULL) {
		printf("Faited to open %s\n", fname);
		return -1;
	}
	sprintf(fname, "%s/%s.clt.c", path, filename);
	fp_clt = fopen(fname, "w");
	if(fp_clt==NULL) {
		printf("Faited to open %s\n", fname);
		return -1;
	}
	sprintf(fname, "%s/%s.svr.c", path, filename);
	fp_svr = fopen(fname, "w");
	if(fp_svr==NULL) {
		printf("Faited to open %s\n", fname);
		return -1;
	}
	
	gen_h(filename);
	gen_client(filename);
	gen_server(filename);
	
	fclose(fp_h);
	fclose(fp_clt);
	fclose(fp_svr);
	
	return 0;
}


int gen_h(const char* filename)
{
	int i, f;
	char str[1000];
	fprintf(fp_h,  "#ifndef _%s_H_\n", filename);
	fprintf(fp_h,  "#define _%s_H_\n", filename);
	fprintf(fp_h,  "\n");
	fprintf(fp_h,  "#ifdef __cplusplus\n");
	fprintf(fp_h,  "extern \"C\" {\n");
	fprintf(fp_h,  "#endif\n");
	fprintf(fp_h,  "\n");
	fprintf(fp_h,  "// C_DEF\n");
	for(i=0; i<i_count; i++) {
		fprintf(fp_h,  "%s\n", &i_list[i][0]);
	}
	fprintf(fp_h,  "\n");
	for(f=0; f<f_count; f++) {
		gen_plist(str, f_list[f].p_list, f_list[f].p_count);
		fprintf(fp_h,  "// %s\n", f_list[f].name);
		fprintf(fp_h,  "int %s(RPCNET_GROUP* group%s%s);\n",
						f_list[f].name,
						strcmp(f_list[f].mode, "async")!=0?"":", RPCFUN_ASYNC_OVERLAPPED* overlap",
						str);
		fprintf(fp_h,  "int %s_impl(RPCNET_GROUP* group%s);\n",
						f_list[f].name,
						str);
	}
	fprintf(fp_h,  "\n");
	fprintf(fp_h,  "extern RPCFUN_FUNCTION_DESC __%s_desc[%d];\n", filename, f_count+1);
	fprintf(fp_h,  "\n");
	fprintf(fp_h,  "#ifdef __cplusplus\n");
	fprintf(fp_h,  "}\n");
	fprintf(fp_h,  "#endif\n");
	fprintf(fp_h,  "\n");
	fprintf(fp_h,  "#endif\n");
	fprintf(fp_h,  "\n");
	return 0;
}

int gen_client(const char* filename)
{
	int f;
	char str[1000];
	fprintf(fp_clt,"#include <stdio.h>\n");
	fprintf(fp_clt,"#include <skates/skates.h>\n");
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"#include \"%s.h\"\n", filename);
	fprintf(fp_clt,"\n");
	for(f=0; f<f_count; f++) {
		gen_plist(str, f_list[f].p_list, f_list[f].p_count);
		fprintf(fp_clt,"int %s(RPCNET_GROUP* group%s%s)\n",
						f_list[f].name,
						strcmp(f_list[f].mode, "async")!=0?"":", RPCFUN_ASYNC_OVERLAPPED* overlap",
						str
						);
		fprintf(fp_clt,"{\n");
		gen_proxy(&f_list[f]);
		fprintf(fp_clt,"}\n");
		fprintf(fp_clt,"\n");
	}
	fprintf(fp_clt,"\n");
	return 0;
}

int gen_server(const char* filename)
{
	int f;
	fprintf(fp_svr,"#include <stdio.h>\n");
	fprintf(fp_svr,"#include <skates/skates.h>\n");
	fprintf(fp_svr,"\n");
	fprintf(fp_svr,"#include \"%s.h\"\n", filename);
	fprintf(fp_svr,"\n");
	for(f=0; f<f_count; f++) {
		fprintf(fp_svr,"static int %s_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* _reserve_conn, RPCFUN_REQUEST_SEQ* _reserve_seq, STREAM* _reserve_in);\n",
						f_list[f].name);
	}
	fprintf(fp_svr,"\n");
	fprintf(fp_svr,"RPCFUN_FUNCTION_DESC __%s_desc[%d] = {\n", filename, f_count+1);
	for(f=0; f<f_count; f++) {
		fprintf(fp_svr,"	{ {{NULL, NULL, NULL}, NULL, 0, 0}, 0, \"%s\", %s_stub, %s_impl },\n",
						f_list[f].name,
						f_list[f].name,
						f_list[f].name);
						
	}
	fprintf(fp_svr,"	{ {{NULL, NULL, NULL}, NULL, 0, 0}, 0, \"\", NULL, NULL },\n");
	fprintf(fp_svr,"};\n");
	fprintf(fp_svr,"\n");
	for(f=0; f<f_count; f++) {
		fprintf(fp_svr,"int %s_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* _reserve_conn, RPCFUN_REQUEST_SEQ* _reserve_seq, STREAM* _reserve_in)\n",
						f_list[f].name);
		fprintf(fp_svr,"{\n");
		gen_stub(&f_list[f]);
		fprintf(fp_svr,"}\n");
		fprintf(fp_svr,"\n");
	}
	fprintf(fp_svr,"\n");
	return 0;
}

void gen_plist(char* str, PARAMETER p_list[], int p_count)
{
	int p;
	str[0] = '\0';
	for(p=0; p<p_count; p++) {
		sprintf(str+strlen(str), ", ");
		sprintf(str+strlen(str), "%s%s%s %s",
			(p_list[p].flag_mode==IDL_MODE_VAL || p_list[p].flag_out!=IDL_FLAG_NONE)?"":"const ",
			p_list[p].type,
			(p_list[p].flag_mode==IDL_MODE_VAL?"":"*"),
			p_list[p].name);
	}
}

void gen_proxy(FUNCTION* func)
{
	int p;
	char str[1000];
	char str_1[1000];

	fprintf(fp_clt,"	struct {\n");
	fprintf(fp_clt,"		RPCNET_THREAD_CONTEXT*		ctx;\n");
	fprintf(fp_clt,"		RPCNET_CONNECTION*			conn;\n");
	fprintf(fp_clt,"		RPCFUN_REQUEST_SEQ				seq;\n");
	fprintf(fp_clt,"		STREAM*							stream;\n");
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"		RPCFUN_PARAMETER_ITEM*			list;\n");
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"		RPCFUN_PARAMETER_ITEM			a_list[%d];\n", func->p_count+1);
	fprintf(fp_clt,"		int								retcode;\n");
	fprintf(fp_clt,"		int								result;\n");
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"		RPCFUN_FUNCTION_DESC*			func_desc;\n");
	fprintf(fp_clt,"	} _reserved;\n");
	fprintf(fp_clt,"\n");

	gen_plist(str, func->p_list, func->p_count);
	str_1[0] = '\0';
	for(p=0; p<func->p_count; p++) {
		sprintf(str_1+strlen(str_1), ", %s", func->p_list[p].name);		
	}
	fprintf(fp_clt,"	if(rpcnet_group_islocal(group)!=0) {\n");
if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"		_reserved.func_desc = rpcfun_get_function(\"%s\");\n", func->name);
	fprintf(fp_clt,"		if(_reserved.func_desc==NULL) return ERR_NOT_IMPLEMENT;\n");
	fprintf(fp_clt,"		if(overlap==NULL) {\n");
	fprintf(fp_clt,"			_reserved.retcode = ((int (*)(RPCNET_GROUP* group%s))_reserved.func_desc->impl)(group%s);\n", str, str_1);
	fprintf(fp_clt,"			rpcfun_release_function(_reserved.func_desc);\n");
	fprintf(fp_clt,"			return(_reserved.retcode);\n");
	fprintf(fp_clt,"		} else {\n");
	fprintf(fp_clt,"			if(overlap==RPCFUN_NOCALLBACK) {\n");
	fprintf(fp_clt,"				_reserved.list = _reserved.a_list;\n");
	fprintf(fp_clt,"			} else {\n");
	fprintf(fp_clt,"				_reserved.list = overlap->_reserved.list;\n");
	fprintf(fp_clt,"			}\n");
	fprintf(fp_clt,"			overlap->_reserved.list_count = %d;\n", func->p_count);
	fprintf(fp_clt,"		}\n");
} else {
#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"%%s (%%s:%%d) start proxy local\", __FUNCTION__, __FILE__, __LINE__);\n");
#endif
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"		_reserved.func_desc = rpcfun_get_function(\"%s\");\n", func->name);
	fprintf(fp_clt,"		if(_reserved.func_desc==NULL) return ERR_NOT_IMPLEMENT;\n");
	fprintf(fp_clt,"		_reserved.retcode = ((int (*)(RPCNET_GROUP* group%s))_reserved.func_desc->impl)(group%s);\n", str, str_1);
	fprintf(fp_clt,"		rpcfun_release_function(_reserved.func_desc);\n");
#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"%%s (%%s:%%d) end proxy local\", __FUNCTION__, __FILE__, __LINE__);\n");
#endif
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"		return(_reserved.retcode);\n");
}	
	fprintf(fp_clt,"	} else {\n");
if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"		_reserved.func_desc = NULL;\n");
	fprintf(fp_clt,"		if(overlap==NULL || overlap==RPCFUN_NOCALLBACK) {\n");
	fprintf(fp_clt,"			_reserved.list = _reserved.a_list;\n");
	fprintf(fp_clt,"		} else {\n");
	fprintf(fp_clt,"			_reserved.list = overlap->_reserved.list;\n");
	fprintf(fp_clt,"			overlap->_reserved.list_count = %d;\n", func->p_count);
	fprintf(fp_clt,"		}\n");
} else {
	fprintf(fp_clt,"		_reserved.list = _reserved.a_list;\n");
}
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"\n");

#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"	DBGLOG(LOG_INFO, \"IDLGEN\", \"%%s (%%s:%%d) start proxy\", __FUNCTION__, __FILE__, __LINE__);\n");
#endif
	fprintf(fp_clt,"\n");
	// step  1: full parameter list\n");
	fprintf(fp_clt,"	// step  1: full parameter list\n");
	for(p=0; p<func->p_count; p++) {
if(strcmp(func->mode, "async")==0 && func->p_list[p].flag_out!=IDL_FLAG_NONE) {
		fprintf(fp_clt,"	_reserved.list[%d].mode = (0%s%s);\n", p,
						func->p_list[p].flag_in==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_IN",
						func->p_list[p].flag_out==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_OUT"
						);
} else {
		fprintf(fp_clt,"	_reserved.list[%d].mode = (0%s%s);\n", p,
						func->p_list[p].flag_in==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_IN",
						func->p_list[p].flag_out==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_OUT"
						);
}
		fprintf(fp_clt,"	_reserved.list[%d].max = %s;\n", p,
						func->p_list[p].flag_out==IDL_FLAG_NONE?"0":func->p_list[p].ptr_len
						);
		fprintf(fp_clt,"	_reserved.list[%d].count = %s;\n", p,
						func->p_list[p].flag_in==IDL_FLAG_NONE?
							(func->p_list[p].flag_mode==IDL_MODE_VAL?"1":"0"):
							(func->p_list[p].flag_in==IDL_FLAG_VAL?func->p_list[p].in_len:func->p_list[p].ptr_len)
						);
		fprintf(fp_clt,"	_reserved.list[%d].size = sizeof(%s);\n", p, func->p_list[p].type);
		fprintf(fp_clt,"	_reserved.list[%d].buf = (%s*)%s%s;\n",
						p,
						func->p_list[p].type,
						func->p_list[p].flag_mode==IDL_MODE_VAL?"&":"",
						func->p_list[p].name);
	}
	fprintf(fp_clt,"	\n");

if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"	// local async call\n");
	fprintf(fp_clt,"	if(overlap!=NULL && _reserved.func_desc!=NULL) {\n");
	fprintf(fp_clt,"		//\n");
	fprintf(fp_clt,"		_reserved.stream = rpcfun_stream_alloc();\n");
	fprintf(fp_clt,"		if(_reserved.stream==NULL) {\n");
	fprintf(fp_clt,"			rpcfun_release_function(_reserved.func_desc);\n");
	fprintf(fp_clt,"			return(ERR_NO_ENOUGH_MEMORY);\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"		//\n");
	fprintf(fp_clt,"		_reserved.retcode = stream_put(_reserved.stream, &_reserved.func_desc, sizeof(_reserved.func_desc));\n");
	fprintf(fp_clt,"		if(_reserved.retcode!=ERR_NOERROR) {\n");
	fprintf(fp_clt,"			rpcfun_release_function(_reserved.func_desc);\n");
	fprintf(fp_clt,"			return(_reserved.retcode);\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"		// serialize\n");
	fprintf(fp_clt,"		_reserved.retcode = rpcfun_serialize_write(_reserved.stream, _reserved.list, %d, NULL);\n", func->p_count);
	fprintf(fp_clt,"		if(_reserved.retcode!=ERR_NOERROR) {\n");
//#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"			DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc serialize write error in %%s, ret=%%d\", __FUNCTION__, _reserved.retcode);\n");
//#endif
	fprintf(fp_clt,"			rpcfun_release_function(_reserved.func_desc);\n");
	fprintf(fp_clt,"			return(_reserved.retcode);\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"		//\n");
	fprintf(fp_clt,"		stream_seek(_reserved.stream, 0);\n");
	fprintf(fp_clt,"		// post\n");
	fprintf(fp_clt,"		_reserved.retcode = rpcfun_localasync_post(_reserved.stream, overlap);\n");
	fprintf(fp_clt,"		if(_reserved.retcode!=ERR_NOERROR){\n");
	fprintf(fp_clt,"			rpcfun_release_function(_reserved.func_desc);\n");
	fprintf(fp_clt,"			return(_reserved.retcode);\n");
	fprintf(fp_clt,"		} else {\n");
	fprintf(fp_clt,"			return(ERR_PENDING);\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"\n");
}

	fprintf(fp_clt,"	// step  2: alloc thread context\n");
	fprintf(fp_clt,"	_reserved.ctx = rpcnet_context_get();\n");
	fprintf(fp_clt,"	if(_reserved.ctx==NULL) return ERR_NOT_WORKER;\n");
	fprintf(fp_clt,"	\n");

	fprintf(fp_clt,"	// step  3: alloc connection\n");
if(strcmp(func->mode, "async")!=0) {
	fprintf(fp_clt,"	_reserved.conn = rpcnet_context_getconn(_reserved.ctx, group);\n");
	fprintf(fp_clt,"	if(_reserved.conn==NULL) {\n");
	fprintf(fp_clt,"		rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"		return ERR_CONNECTION_FAILED;\n");
	fprintf(fp_clt,"		}\n");
} else {
	fprintf(fp_clt,"	if(overlap==NULL) {\n");
	fprintf(fp_clt,"		_reserved.conn = rpcnet_context_getconn(_reserved.ctx, group);\n");
	fprintf(fp_clt,"		if(_reserved.conn==NULL) {\n");
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return ERR_CONNECTION_FAILED;\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"	} else {\n");
	fprintf(fp_clt,"		_reserved.conn = rpcnet_context_getconn_force(_reserved.ctx, group);\n");
	fprintf(fp_clt,"		if(_reserved.conn==NULL) {\n");
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return ERR_CONNECTION_FAILED;\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"	}\n");
}
	fprintf(fp_clt,"	\n");

	fprintf(fp_clt,"	// step  4: get stream\n");
	fprintf(fp_clt,"	_reserved.stream = rpcnet_context_getstream(_reserved.ctx);\n");
	fprintf(fp_clt,"	if(_reserved.stream==NULL) {\n");
	fprintf(fp_clt,"		rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"		return ERR_UNKNOWN;\n");
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"	\n");

if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"	// if async_rpc then ** async call\n");
	fprintf(fp_clt,"	// step  5: register request sequse\n");
	fprintf(fp_clt,"	// endif\n");
	fprintf(fp_clt,"	if(overlap!=NULL && overlap!=RPCFUN_NOCALLBACK) {\n");
	fprintf(fp_clt,"		_reserved.retcode = rpcfun_register_overlap(group, overlap, &_reserved.seq);\n");
	fprintf(fp_clt,"		if(_reserved.retcode!=ERR_NOERROR) {\n");
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return _reserved.retcode;\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"	\n");
}

	fprintf(fp_clt,"	// step  6: write header to stream\n");
if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"	_reserved.retcode = rpcfun_write_head(_reserved.stream, \"%s\", (overlap==NULL || overlap==RPCFUN_NOCALLBACK)?NULL:&_reserved.seq);\n", func->name);
} else {
	fprintf(fp_clt,"	_reserved.retcode = rpcfun_write_head(_reserved.stream, \"%s\", NULL);\n", func->name);
}
	fprintf(fp_clt,"	if(_reserved.retcode!=ERR_NOERROR) {\n");
#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc write head error in %%s, ret=%%d\", __FUNCTION__, _reserved.retcode);\n");
#endif
	fprintf(fp_clt,"		rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"		return _reserved.retcode;\n");
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"	\n");

	fprintf(fp_clt,"	// step  7: write parameter to stream\n");
	fprintf(fp_clt,"	_reserved.retcode = rpcfun_serialize_write(_reserved.stream, _reserved.list, %d, NULL);\n", func->p_count);
	fprintf(fp_clt,"	if(_reserved.retcode!=ERR_NOERROR) {\n");
//#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc serialize write error in %%s, ret=%%d\", __FUNCTION__, _reserved.retcode);\n");
//#endif
	fprintf(fp_clt,"		rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"		return _reserved.retcode;\n");
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"	\n");

	fprintf(fp_clt,"	// step  8: send stream\n");
if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"	_reserved.retcode = rpcnet_conn_write(_reserved.conn,	overlap==NULL?RPCNET_SUBSYS_SYNCRPC:\n");
	fprintf(fp_clt,"							(overlap==RPCFUN_NOCALLBACK?RPCNET_SUBSYS_ASYNCRPC_NOCALLBACK:\n");
	fprintf(fp_clt,"							RPCNET_SUBSYS_ASYNCRPC_REQUEST), _reserved.stream);\n");
} else{
	fprintf(fp_clt,"	_reserved.retcode = rpcnet_conn_write(_reserved.conn, RPCNET_SUBSYS_SYNCRPC, _reserved.stream);\n");
}
	fprintf(fp_clt,"	if(_reserved.retcode!=ERR_NOERROR) {\n");
	fprintf(fp_clt,"		rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"		return _reserved.retcode;\n");
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"	\n");

if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"	// if sync_rpc then ** sync call\n");
	fprintf(fp_clt,"	if(overlap==NULL) {\n");
}
	fprintf(fp_clt,"		// step  9: get stream\n");
	fprintf(fp_clt,"		_reserved.stream = rpcnet_context_getstream(_reserved.ctx);\n");
	fprintf(fp_clt,"		if(_reserved.stream==NULL) {\n");
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return ERR_UNKNOWN;\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"		\n");

	fprintf(fp_clt,"		// step 10: receive stream\n");
	fprintf(fp_clt,"		_reserved.retcode = rpcnet_conn_read(group, _reserved.conn, RPCNET_SUBSYS_SYNCRPC_RESPONSE, _reserved.stream);\n");
	fprintf(fp_clt,"		if(_reserved.retcode!=ERR_NOERROR) {\n");
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return _reserved.retcode;\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"		\n");

	fprintf(fp_clt,"		// step 11: read result from stream\n");
	fprintf(fp_clt,"		_reserved.retcode = rpcfun_read_result(_reserved.stream, NULL, &_reserved.result);\n");
	fprintf(fp_clt,"		if(_reserved.retcode!=ERR_NOERROR) {\n");
#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"			DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc read result error in %%s, ret=%%d\", __FUNCTION__, _reserved.retcode);\n");
#endif
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return _reserved.retcode;\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"		if(_reserved.result!=ERR_NOERROR) {\n");
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return _reserved.result;\n");
	fprintf(fp_clt,"		}\n");
	fprintf(fp_clt,"		\n");

	fprintf(fp_clt,"		// step 12: read parameter from stream\n");
	fprintf(fp_clt,"		_reserved.retcode = rpcfun_serialize_read(_reserved.stream, _reserved.list, %d, NULL);\n", func->p_count);
	fprintf(fp_clt,"		if(_reserved.retcode!=ERR_NOERROR) {\n");
//#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"			DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc serialize read error in %%s, ret=%%d\", __FUNCTION__, _reserved.retcode);\n");
//#endif
	fprintf(fp_clt,"			rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"			return _reserved.retcode;\n");
	fprintf(fp_clt,"		}\n");
if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"	} else {\n");
	fprintf(fp_clt,"		rpcnet_context_freeconn(_reserved.ctx, _reserved.conn);\n");
	fprintf(fp_clt,"	}\n");
	fprintf(fp_clt,"	// endif\n");
}
	fprintf(fp_clt,"	\n");

	fprintf(fp_clt,"	// step 12: free thread context\n");
	fprintf(fp_clt,"	rpcnet_context_free(_reserved.ctx);\n");
	fprintf(fp_clt,"	\n");

#ifdef OUTPUT_DEBUG
	fprintf(fp_clt,"	DBGLOG(LOG_INFO, \"IDLGEN\", \"%%s (%%s:%%d) end proxy\", __FUNCTION__, __FILE__, __LINE__);\n");
#endif
	fprintf(fp_clt,"\n");
	fprintf(fp_clt,"	// step 13: return RETCODE\n");
if(strcmp(func->mode, "async")==0) {
	fprintf(fp_clt,"	return (overlap==NULL || overlap==RPCFUN_NOCALLBACK)?ERR_NOERROR:ERR_PENDING;\n");
} else {
	fprintf(fp_clt,"	return ERR_NOERROR;\n");
}
}

void gen_stub(FUNCTION* func)
{
	int p;

	fprintf(fp_svr,"	// define parameter\n");	
	for(p=0; p<func->p_count; p++) {
		fprintf(fp_svr,"	%s%s %s;\n", func->p_list[p].type,
						func->p_list[p].flag_mode==IDL_MODE_VAL?"":"*",
						func->p_list[p].name);
	}
	fprintf(fp_svr,"	// reserve\n");
	fprintf(fp_svr,"	int _reserve_ret;\n");
	fprintf(fp_svr,"	STREAM* _reserve_stream = NULL;\n");
	if(func->p_count>0) {
		fprintf(fp_svr,"	RPCFUN_PARAMETER_ITEM	_reserve_list[%d];\n", func->p_count);
	} else {
		fprintf(fp_svr,"	RPCFUN_PARAMETER_ITEM	_reserve_list[%d];\n", 1);
	}
	fprintf(fp_svr,"\n");

#ifdef OUTPUT_DEBUG
	fprintf(fp_svr,"	DBGLOG(LOG_INFO, \"IDLGEN\", \"%%s (%%s:%%d) start stub\", __FUNCTION__, __FILE__, __LINE__);\n");
#endif
	fprintf(fp_svr,"\n");
	fprintf(fp_svr,"	// step  1: fill parameter list\n");
	for(p=0; p<func->p_count; p++) {
if(strcmp(func->mode, "async")==0 && func->p_list[p].flag_out!=IDL_FLAG_NONE) {
		fprintf(fp_svr,"	_reserve_list[%d].mode = (0%s%s);\n", p,
						func->p_list[p].flag_in==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_IN",
						func->p_list[p].flag_out==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_OUT"
						);
} else {
		fprintf(fp_svr,"	_reserve_list[%d].mode = (0%s%s);\n", p,
						func->p_list[p].flag_in==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_IN",
						func->p_list[p].flag_out==IDL_FLAG_NONE?"":"|RPCFUN_SERIALIZE_OUT"
						);
}
		fprintf(fp_svr,"	_reserve_list[%d].max = 0;\n", p);
		fprintf(fp_svr,"	_reserve_list[%d].count = 1;\n", p);
		fprintf(fp_svr,"	_reserve_list[%d].size = sizeof(%s);\n", p, func->p_list[p].type);
		fprintf(fp_svr,"	_reserve_list[%d].buf = %s%s;\n", p,
						func->p_list[p].flag_mode==IDL_MODE_VAL?"&":"",
						func->p_list[p].flag_mode==IDL_MODE_VAL?func->p_list[p].name:"NULL"
						);
	}
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// step  2: read parameter from stream\n");
	fprintf(fp_svr,"	_reserve_ret = rpcfun_serialize_read(_reserve_in, _reserve_list, %d, ctx);\n", func->p_count);
	fprintf(fp_svr,"	if(_reserve_ret!=ERR_NOERROR) {\n");
//#ifdef OUTPUT_DEBUG
	fprintf(fp_svr,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc serialize read error in %%s, ret=%%d\", __FUNCTION__, _reserve_ret);\n");
//#endif
	fprintf(fp_svr,"		if(_reserve_conn==NULL && _reserve_seq==NULL) {\n");
	fprintf(fp_svr,"			return(_reserve_ret);\n");
	fprintf(fp_svr,"		} else {\n");
	fprintf(fp_svr,"			goto SEND_ERROR;\n");
	fprintf(fp_svr,"		}\n");
	fprintf(fp_svr,"	}\n");
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// step  3: fill pointer\n");
	for(p=0; p<func->p_count; p++) {
		if(func->p_list[p].flag_mode==IDL_MODE_PTR) fprintf(fp_svr,"	%s = (%s*)_reserve_list[%d].buf;\n", func->p_list[p].name, func->p_list[p].type, p);
	}
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// step  4: call impl\n");
	fprintf(fp_svr,"	_reserve_ret = %s_impl(group", func->name);
	for(p=0; p<func->p_count; p++) fprintf(fp_svr,", %s", func->p_list[p].name);
	fprintf(fp_svr,");\n");
	fprintf(fp_svr,"	if(_reserve_conn==NULL && _reserve_seq==NULL) return(_reserve_ret);\n");
	fprintf(fp_svr,"	if(_reserve_ret!=ERR_NOERROR) goto SEND_ERROR;\n");
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// step  5: fill parameter\n");
	for(p=0; p<func->p_count; p++) {
		if(func->p_list[p].flag_out==IDL_FLAG_NONE) continue;
		fprintf(fp_svr,"	_reserve_list[%d].count = %s;\n", p, func->p_list[p].flag_out==IDL_FLAG_PTR?func->p_list[p].ptr_len:func->p_list[p].out_len);
	}
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// step  6: get stream\n");
	fprintf(fp_svr,"	_reserve_stream = rpcnet_context_getstream(ctx);\n");
	fprintf(fp_svr,"	if(_reserve_stream==NULL) goto SEND_ERROR;\n");
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// step  7: write result\n");
	fprintf(fp_svr,"	_reserve_ret = rpcfun_write_result(_reserve_stream, _reserve_seq, _reserve_ret);\n");
	fprintf(fp_svr,"	if(_reserve_ret!=ERR_NOERROR) {\n");
#ifdef OUTPUT_DEBUG
	fprintf(fp_svr,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc write result error in %s, ret=%d\", __FUNCTION__, _reserve_ret);\n");
#endif
	fprintf(fp_svr,"		goto SEND_ERROR;\n");
	fprintf(fp_svr,"	}\n");
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// step  8: write parameter to stream\n");
	fprintf(fp_svr,"	_reserve_ret = rpcfun_serialize_write(_reserve_stream, _reserve_list, %d, ctx);\n", func->p_count);
	fprintf(fp_svr,"	if(_reserve_ret!=ERR_NOERROR) {\n");
//#ifdef OUTPUT_DEBUG
	fprintf(fp_svr,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc serialize write error in %%s, ret=%%d\", __FUNCTION__, _reserve_ret);\n");
//#endif
	fprintf(fp_svr,"		goto SEND_ERROR;\n");
	fprintf(fp_svr,"	}\n");
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"	// if async_call then\n");
	fprintf(fp_svr,"	if(_reserve_seq!=NULL) {\n");
	fprintf(fp_svr,"		// step  9: alloc connection\n");
	fprintf(fp_svr,"		_reserve_conn = rpcnet_context_getconn(ctx, group);\n");
	fprintf(fp_svr,"		if(_reserve_conn==NULL) return(ERR_UNKNOWN);\n");
	fprintf(fp_svr,"	}\n");
	fprintf(fp_svr,"	// endif\n");
	fprintf(fp_svr,"\n");
	
	fprintf(fp_svr,"	// step 10: send stream;\n");
	fprintf(fp_svr,"	_reserve_ret = rpcnet_conn_write(_reserve_conn, _reserve_seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, _reserve_stream);\n");
	fprintf(fp_svr,"	if(_reserve_ret!=ERR_NOERROR) return(_reserve_ret);\n");
	fprintf(fp_svr,"\n");

#ifdef OUTPUT_DEBUG
	fprintf(fp_svr,"	DBGLOG(LOG_INFO, \"IDLGEN\", \"%%s (%%s:%%d) end stub\", __FUNCTION__, __FILE__, __LINE__);\n");
#endif
	fprintf(fp_svr,"\n");
	fprintf(fp_svr,"	// done.\n");
	fprintf(fp_svr,"	return(ERR_NOERROR);\n");
	fprintf(fp_svr,"\n");

	fprintf(fp_svr,"SEND_ERROR:\n");
	fprintf(fp_svr,"	if(_reserve_conn==NULL) {\n");
	fprintf(fp_svr,"		_reserve_conn = rpcnet_context_getconn(ctx, group);\n");
	fprintf(fp_svr,"		if(_reserve_conn==NULL) return(ERR_UNKNOWN);\n");
	fprintf(fp_svr,"	}\n");
	fprintf(fp_svr,"	if(_reserve_stream==NULL) {\n");
	fprintf(fp_svr,"		_reserve_stream = rpcnet_context_getstream(ctx);\n");
	fprintf(fp_svr,"		if(_reserve_stream==NULL) return(ERR_UNKNOWN);\n");
	fprintf(fp_svr,"	} else {\n");
	fprintf(fp_svr,"		stream_clear(_reserve_stream);\n");
	fprintf(fp_svr,"	}\n");	
	fprintf(fp_svr,"	_reserve_ret = rpcfun_write_result(_reserve_stream, _reserve_seq, _reserve_ret);\n");
	fprintf(fp_svr,"	if(_reserve_ret!=ERR_NOERROR) {\n");
#ifdef OUTPUT_DEBUG
	fprintf(fp_svr,"		DBGLOG(LOG_INFO, \"IDLGEN\", \"rpc write result error in %%s, ret=%%d\", __FUNCTION__, _reserve_ret);\n");
#endif
	fprintf(fp_svr,"		return(_reserve_ret);\n");
	fprintf(fp_svr,"	}\n");
	fprintf(fp_svr,"	_reserve_ret = rpcnet_conn_write(_reserve_conn, _reserve_seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, _reserve_stream);\n");
	fprintf(fp_svr,"	if(_reserve_ret!=ERR_NOERROR) {\n");
	fprintf(fp_svr,"		return(_reserve_ret);\n");
	fprintf(fp_svr,"	}\n");
	fprintf(fp_svr,"	return(ERR_NOERROR);\n");
	
}

