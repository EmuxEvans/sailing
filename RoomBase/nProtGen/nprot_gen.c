#include <time.h>
#include <stdio.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

#include "..\..\framework\tools\prot_parser.h"

static char txt[50*1024];
static char type[100], prelen[100], name[100], max[2000];

static int parser_nfile(const char* buf);
static int generate_pfile(const char* name, char* inc, unsigned int inc_len);
static int generate_hcltfile(const char* name, char* inc, unsigned int inc_len);
static int generate_ccltfile(const char* name, char* src, unsigned int src_len);
static int generate_hsvrfile(const char* name, char* inc, unsigned int inc_len);
static int generate_csvrfile(const char* name, char* src, unsigned int src_len);

static const char* parse_include(const char* buf);
static const char* parse_module(const char* buf);
static const char* parse_function(const char* buf);
static const char* parse_parameter(const char* buf);

static void def_include(const char* name);
static void def_module(const char* name);
static void def_function(const char* type, const char* name);
static void def_parameter(const char* type, const char* prelen, const char* name, const char* max);

typedef struct NDATA_MODULE {
	char	name[100];
	int		f_start, f_count;
} NDATA_MODULE;

typedef struct NDATA_FUNCTION {
	char	type; // 'C' or 'S'
	char	name[100];
	int		p_start, p_count;
} NDATA_FUNCTION;

typedef struct NDATA_PARAMETER {
	char	type[100];
	char	prelen[100];
	char	name[100];
	char	max[100];
} NDATA_PARAMETER;

static char mode = 0;

static NDATA_MODULE nmodules[100];
static int nmodules_count = 0;
static NDATA_FUNCTION nfunctions[1000];
static int nfunctions_count = 0;
static NDATA_PARAMETER nparams[1000];
static int nparams_count = 0;

int main(int argc, char* argv[])
{
	int ret;
	char file[200];

	if(argc<3) {
		printf("invalid parameter\n");
		return 0;
	}

	if(strcmp(argv[1], "client")!=0 && strcmp(argv[1], "server")!=0) {
		printf("invalid type\n");
		return 0;
	}
	mode = strcmp(argv[1], "client")==0?'C':'S';

	if(load_textfile(argv[2], txt, sizeof(txt))<=0) {
		printf("can't open %s\n", argv[2]);
		return 0;
	}

	{
		char opath[200], path[200];
		char *a, *b;

		os_getcwd(opath, sizeof(opath));
		strcpy(path, argv[2]);
		a = strrchr(path, '\\');
		b = strrchr(path, '/');
		if(b>a) a = b;
		if(a!=NULL) {
			*a = '\0';
			os_chdir(path);
		}
		ret = parser_nfile(txt);
		os_chdir(opath);
	}

	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}

	memset(txt, 0, sizeof(txt));
	ret = generate_pfile(argv[2], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.proto", argv[2]);
	save_textfile(file, txt, 0);

	memset(txt, 0, sizeof(txt));
	ret = generate_hcltfile(argv[2], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.clt.hpp", argv[2]);
	save_textfile(file, txt, 0);

	memset(txt, 0, sizeof(txt));
	ret = generate_ccltfile(argv[2], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.clt.cpp", argv[2]);
	save_textfile(file, txt, 0);

	memset(txt, 0, sizeof(txt));
	ret = generate_hsvrfile(argv[2], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.svr.hpp", argv[2]);
	save_textfile(file, txt, 0);

	memset(txt, 0, sizeof(txt));
	ret = generate_csvrfile(argv[2], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.svr.cpp", argv[2]);
	save_textfile(file, txt, 0);

	return 0;
}

int parser_nfile(const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	for(;;) {
		if(is_break) {
			return ERR_UNKNOWN;
		}

		buf = escape_blank(tbuf);
		if(*buf=='\0') break;

		tbuf = parse_include(buf);
		if(tbuf) continue;

		tbuf = parse_module(buf);
		if(tbuf!=NULL) continue;

		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

static void print_pdef(int c, int f, char* txt, unsigned int txt_len)
{
	int p;
	for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
		if(p>nfunctions[f].p_start) {
			snprintf(txt+strlen(txt), txt_len-strlen(txt), ", ");
		}
		snprintf(txt+strlen(txt), txt_len-strlen(txt), ", ");
	}
}

int generate_pfile(const char* name, char* inc, unsigned int inc_len)
{
	struct tm   *newTime;
    time_t      szClock;
	int i, c, f, p;

	inc[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	inc[0] = '\0';
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// generate by NPROT_GEN.\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// %s\n", asctime(newTime));
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	for(i=0; i<num_inc; i++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "include %s;\n", data_include[i].file);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	for(c=0; c<nmodules_count; c++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "interface I%sClient {\n", nmodules[c].name);
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='C') continue;
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	void %s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", nparams[p].type, nparams[p].name);
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ");\n");
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "interface I%sServer {\n", nmodules[c].name);
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='S') continue;
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	void %s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", nparams[p].type, nparams[p].name);
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ");\n");
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	}

	return ERR_NOERROR;
}

int generate_hcltfile(const char* name, char* inc, unsigned int inc_len)
{
	struct tm   *newTime;
    time_t      szClock;
	int c, f, p;
	char buf[1000];

	inc[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	inc[0] = '\0';
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// generate by NPROT_GEN.\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// %s\n", asctime(newTime));
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	make_define_filename(name, buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifndef __%s_CLIENT_INCLUDE__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define __%s_CLIENT_INCLUDE__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	for(c=0; c<nmodules_count; c++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class C%sClientHook : public I%sClient {\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "protected:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sClientHook(const char* pName) {\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "		m_bHandled = false;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "		m_pName = pName;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	}\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	~C%sClientHook() {}\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "private:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	const char* m_pName;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	void SetHandled(bool bHandled=true) { m_bHandled = bHandled; }\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool IsHandled() { return m_bHandled; };\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool m_bHandled;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='C') continue;
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual void %s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ") {}\n");
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class C%sClientBase : public I%sServer {\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sClientBase(bool bTextMode, C%sClientHook** pHooks, int nHookMax);\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	~C%sClientBase();\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool IsTextMode() const { return m_bTextMode; }\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	void SetTextMode(bool bTextMode=false) { m_bTextMode = bTextMode; }\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool Dispatch(const void* pData, unsigned int nSize);\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "protected:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual char* GetSendBuf(unsigned int& nSendBufSize) = NULL;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual void SendBuf(const char* pSendBuf, unsigned int nSendBufSize) = NULL;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "private:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool				m_bTextMode;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sClientHook** m_pHooks;\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	int					m_nHookMax;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='S') continue;
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual void %s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ");\n");
		}

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	}

	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	return ERR_NOERROR;
}

int generate_ccltfile(const char* name, char* src, unsigned int src_len)
{
	struct tm   *newTime;
    time_t      szClock;
	int c, f, p;

	src[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	src[0] = '\0';
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// generate by NPROT_GEN.\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// %s\n", asctime(newTime));
	snprintf(src+strlen(src), src_len-strlen(src), "#include <assert.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/skates.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.proto.h\"\n", name);
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.clt.hpp\"\n", name);

	for(c=0; c<nmodules_count; c++) {
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			int count = 0;
			if(nfunctions[f].type!='C') continue;

			snprintf(src+strlen(src), src_len-strlen(src), "static bool %s_%s(bool bTextMode, C%sClientHook** pHooks, int nHookCount, const void* pData, unsigned int nSize)\n", nmodules[c].name, nfunctions[f].name, nmodules[c].name);
			snprintf(src+strlen(src), src_len-strlen(src), "{\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	int ret;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	unsigned data_len;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	struct {\n");
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(get_basetype(nparams[p].type)==PROTOCOL_TYPE_STRING) {
					snprintf(src+strlen(src), src_len-strlen(src), "		char %s[%s+1];\n", nparams[p].name, nparams[p].prelen);
				} else {
					snprintf(src+strlen(src), src_len-strlen(src), "		%s %s;\n", nparams[p].type, nparams[p].name);
				}
			}
			snprintf(src+strlen(src), src_len-strlen(src), "	} v_params;\n");

			if(nfunctions[f].p_count>0) {
			snprintf(src+strlen(src), src_len-strlen(src), "	static PROTOCOL_VARIABLE v_list[] = {\n");
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				char stype[100], obj_type[100], prelen[100];
				sprintf(stype, "%s", get_type_const(nparams[p].type));
				if(get_basetype(nparams[p].type)==PROTOCOL_TYPE_STRING) {
					sprintf(prelen, "%s+1", nparams[p].prelen);
				} else {
					sprintf(prelen, "sizeof(%s)", nparams[p].type);
				}
				if(get_basetype(nparams[p].type)==0) {
					sprintf(obj_type, "&PROTOCOL_NAME(%s)", nparams[p].type);
				} else {
					sprintf(obj_type, "NULL");
				}
				snprintf(src+strlen(src), src_len-strlen(src), "		{\"%s\", %s, %s, %s, %s, (unsigned int)((char*)(&v_params.%s)-(char*)&v_params)},\n",
					nparams[p].name, stype, obj_type, prelen, "0", nparams[p].name);
				count++;
			}
			snprintf(src+strlen(src), src_len-strlen(src), "	};\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	static PROTOCOL_TYPE v_type = {\"%s.%s\", v_list, %d, sizeof(%s), {\"\", PROTOCOL_TYPE_FAKEVAR|PROTOCOL_TYPE_STRUCT, &v_type, sizeof(\"v_params\"), 0, 0}};\n", nmodules[c].name, nfunctions[f].name, count, "v_params");
			} else {
			snprintf(src+strlen(src), src_len-strlen(src), "	static PROTOCOL_TYPE v_type = {\"%s.%s\", NULL, %d, sizeof(%s), {\"\", PROTOCOL_TYPE_FAKEVAR|PROTOCOL_TYPE_STRUCT, &v_type, sizeof(\"v_params\"), 0, 0}};\n", nmodules[c].name, nfunctions[f].name, count, "v_params");
			}


			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	data_len = nSize;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	if(bTextMode) {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		ret = protocol_text_read(&v_type, \"%s\", (const char*)pData, &data_len, &v_params);\n", nparams[p].name);
			snprintf(src+strlen(src), src_len-strlen(src), "	} else {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		ret = protocol_binary_read(&v_type, pData, &data_len, &v_params);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	if(ret!=ERR_NOERROR)\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		return false;\n");

			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	for(ret=0; ret<nHookCount; ret++) {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		if(pHooks[ret]==NULL) continue;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		pHooks[ret]->%s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p!=nfunctions[f].p_start)
					snprintf(src+strlen(src), src_len-strlen(src), ", ");

				if(get_basetype(nparams[p].type)==0) {
					snprintf(src+strlen(src), src_len-strlen(src), "&v_params.%s", nparams[p].name);
				} else {
					snprintf(src+strlen(src), src_len-strlen(src), "v_params.%s", nparams[p].name);
				}
			}
			snprintf(src+strlen(src), src_len-strlen(src), ");\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	return true;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
		}

		snprintf(src+strlen(src), src_len-strlen(src), "C%sClientBase::C%sClientBase(bool bTextMode, C%sClientHook** pHooks, int nHookMax)\n", nmodules[c].name, nmodules[c].name, nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	m_bTextMode = bTextMode;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	m_pHooks = pHooks;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	m_nHookMax = nHookMax;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "C%sClientBase::~C%sClientBase()\n", nmodules[c].name, nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "bool C%sClientBase::Dispatch(const void* pData, unsigned int nSize)\n", nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	if(m_bTextMode) {\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		const char* end;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		char name[100];\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		end = strchr((const char*)pData, '{');\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		assert(end-(const char*)pData<sizeof(name)-1);\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		if(end-(const char*)pData>sizeof(name)) return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		memcpy(name, pData, end-(const char*)pData);\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		name[end-(const char*)pData] = '\\0';\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		strtrim(strltrim(name));\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='C') continue;
			snprintf(src+strlen(src), src_len-strlen(src), "		if(strcmp(name, \"%s\")==0) {\n", nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "			return %s_%s(true, m_pHooks, m_nHookMax, (const char*)pData, nSize);\n", nmodules[c].name, nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
		}
		snprintf(src+strlen(src), src_len-strlen(src), "		return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	} else {\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		if(nSize==0) return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		switch(*((const unsigned char*)pData)) {\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='C') continue;
			snprintf(src+strlen(src), src_len-strlen(src), "		case %d: // %s\n", f-nmodules[c].f_start, nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "			return %s_%s(false, m_pHooks, m_nHookMax, (const char*)pData+1, nSize-1);\n", nmodules[c].name, nfunctions[f].name);
		}
		snprintf(src+strlen(src), src_len-strlen(src), "		default:\n");
		snprintf(src+strlen(src), src_len-strlen(src), "			return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");

		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='S') continue;
			snprintf(src+strlen(src), src_len-strlen(src), "void C%sClientBase::%s(", nmodules[c].name, nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(src+strlen(src), src_len-strlen(src), ", ");
				snprintf(src+strlen(src), src_len-strlen(src), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
			}
			snprintf(src+strlen(src), src_len-strlen(src), ")\n");
			snprintf(src+strlen(src), src_len-strlen(src), "{\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	int m_nlen;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	unsigned int nSendBufSize;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	char* pSendBuf;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	pSendBuf = GetSendBuf(nSendBufSize);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	if(m_bTextMode) {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen = 0;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s {\" );\n", nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		\n");

			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				switch(get_basetype(nparams[p].type)) {
				case PROTOCOL_TYPE_CHAR:
				case PROTOCOL_TYPE_SHORT:
				case PROTOCOL_TYPE_INT:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%d;\", (int)%s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_LONG:
#ifdef _WIN32
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%I64d;\", %s);\n", nparams[p].name, nparams[p].name);
#else
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%lld;\", %s);\n", nparams[p].name, nparams[p].name);
#endif
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_BYTE:
				case PROTOCOL_TYPE_WORD:
				case PROTOCOL_TYPE_DWORD:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%u;\", (unsigned int)%s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_QWORD:
#ifdef _WIN32
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%I64u;\", %s);\n", nparams[p].name, nparams[p].name);
#else
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%llu;\", %s);\n", nparams[p].name, nparams[p].name);
#endif
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_FLOAT:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%f;\", %s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_STRING:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=\\\"%%s\\\";\", %s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				default:
					snprintf(src+strlen(src), src_len-strlen(src), "		{\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			int ____ret, ____len;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____len = nSendBufSize-m_nlen;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____ret = protocol_text_write(&PROTOCOL_NAME(%s), \"%s=\", %s, pSendBuf+m_nlen, (unsigned int*)&____len);\n", nparams[p].type, nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "			assert(____ret==ERR_NOERROR);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			if(____ret!=ERR_NOERROR) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			if(m_nlen+(unsigned int)____len>=nSendBufSize) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			pSendBuf[m_nlen+____len] = ';';\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			m_nlen += ____len + 1;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
					break;
				}
			}

			snprintf(src+strlen(src), src_len-strlen(src), "		\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"}\");\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen++;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	} else {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen = 1;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		*((os_byte*)pSendBuf) = %d;\n", f-nmodules[c].f_start);
			snprintf(src+strlen(src), src_len-strlen(src), "\n");

			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start)
					snprintf(src+strlen(src), src_len-strlen(src), "\n");

				switch(get_basetype(nparams[p].type)) {
				case PROTOCOL_TYPE_CHAR:
				case PROTOCOL_TYPE_SHORT:
				case PROTOCOL_TYPE_INT:
				case PROTOCOL_TYPE_LONG:
				case PROTOCOL_TYPE_BYTE:
				case PROTOCOL_TYPE_WORD:
				case PROTOCOL_TYPE_DWORD:
				case PROTOCOL_TYPE_QWORD:
				case PROTOCOL_TYPE_FLOAT:
					snprintf(src+strlen(src), src_len-strlen(src), "		assert((int)nSendBufSize-m_nlen>=sizeof(%s));\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		if((int)nSendBufSize-m_nlen<sizeof(%s)) return;\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		*((%s*)(pSendBuf+m_nlen)) = %s;\n", nparams[p].type, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += sizeof(%s);\n", nparams[p].name);
					break;
				case PROTOCOL_TYPE_STRING:
					snprintf(src+strlen(src), src_len-strlen(src), "		assert((int)nSendBufSize-m_nlen>=(int)sizeof(short));\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if((int)nSendBufSize-m_nlen<(int)sizeof(short)) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		*((os_word*)(pSendBuf+m_nlen)) = (os_word)strlen(%s);\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert((int)nSendBufSize-m_nlen>=(int)sizeof(os_word)+*((short*)(pSendBuf+m_nlen)));\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if((int)nSendBufSize-m_nlen<(int)sizeof(os_word)+*((short*)(pSendBuf+m_nlen))) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		memcpy(pSendBuf+m_nlen+sizeof(os_word), %s, (size_t)(*((short*)(pSendBuf+m_nlen))));\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += sizeof(os_word) + *((os_word*)(pSendBuf+m_nlen));\n");
					break;
				default:
					snprintf(src+strlen(src), src_len-strlen(src), "		{\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			int ____ret, ____len;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____len = nSendBufSize - m_nlen;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____ret = protocol_binary_write(&PROTOCOL_NAME(%s), %s, pSendBuf+m_nlen, (unsigned int *)&____len);\n", nparams[p].type, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "			assert(____ret==ERR_NOERROR);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			m_nlen += ____len;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
					break;
				}
			}


			snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	SendBuf(pSendBuf, (unsigned int)m_nlen);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
		}
	}
	snprintf(src+strlen(src), src_len-strlen(src), "// END OF FILE\n");

	return ERR_NOERROR;
}

int generate_hsvrfile(const char* name, char* inc, unsigned int inc_len)
{
	struct tm   *newTime;
    time_t      szClock;
	int c, f, p;
	char buf[1000];

	inc[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	inc[0] = '\0';
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// generate by NPROT_GEN.\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// %s\n", asctime(newTime));
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	make_define_filename(name, buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifndef __%s_SERVER_INCLUDE__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define __%s_SERVER_INCLUDE__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	for(c=0; c<nmodules_count; c++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class C%sServerHook : public I%sServer {\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "protected:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sServerHook(const char* pName) {\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "		m_bHandled = false;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "		m_pName = pName;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	}\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	~C%sServerHook() {}\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "private:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	const char* m_pName;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	void SetHandled(bool bHandled=true) { m_bHandled = bHandled; }\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool IsHandled() { return m_bHandled; };\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool m_bHandled;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='S') continue;
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual void %s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ") {}\n");
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class C%sServerBase : public I%sClient {\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sServerBase(bool bTextMode, C%sServerHook** pHooks, int nHookMax);\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	~C%sServerBase();\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool IsTextMode() const { return m_bTextMode; }\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	void SetTextMode(bool bTextMode=false) { m_bTextMode = bTextMode; }\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool Dispatch(const void* pData, unsigned int nSize);\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "protected:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual char* GetSendBuf(unsigned int& nSendBufSize) = NULL;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual void SendBuf(const char* pSendBuf, unsigned int nSendBufSize) = NULL;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "private:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool				m_bTextMode;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sServerHook** m_pHooks;\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	int					m_nHookMax;\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='C') continue;
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual void %s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ");\n");
		}

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	}

	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	return ERR_NOERROR;
}

int generate_csvrfile(const char* name, char* src, unsigned int src_len)
{
	struct tm   *newTime;
    time_t      szClock;
	int c, f, p;

	src[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	src[0] = '\0';
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// generate by NPROT_GEN.\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// %s\n", asctime(newTime));
	snprintf(src+strlen(src), src_len-strlen(src), "#include <assert.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/skates.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.proto.h\"\n", name);
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.svr.hpp\"\n", name);

	for(c=0; c<nmodules_count; c++) {
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			int count = 0;
			if(nfunctions[f].type!='S') continue;

			snprintf(src+strlen(src), src_len-strlen(src), "static bool %s_%s(bool bTextMode, C%sServerHook** pHooks, int nHookCount, const void* pData, unsigned int nSize)\n", nmodules[c].name, nfunctions[f].name, nmodules[c].name);
			snprintf(src+strlen(src), src_len-strlen(src), "{\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	int ret;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	unsigned data_len;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	struct {\n");
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(get_basetype(nparams[p].type)==PROTOCOL_TYPE_STRING) {
					snprintf(src+strlen(src), src_len-strlen(src), "		char %s[%s+1];\n", nparams[p].name, nparams[p].prelen);
				} else {
					snprintf(src+strlen(src), src_len-strlen(src), "		%s %s;\n", nparams[p].type, nparams[p].name);
				}
			}
			snprintf(src+strlen(src), src_len-strlen(src), "	} v_params;\n");

			if(nfunctions[f].p_count>0) {
			snprintf(src+strlen(src), src_len-strlen(src), "	static PROTOCOL_VARIABLE v_list[] = {\n");
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				char stype[100], obj_type[100], prelen[100];
				sprintf(stype, "%s", get_type_const(nparams[p].type));
				if(get_basetype(nparams[p].type)==PROTOCOL_TYPE_STRING) {
					sprintf(prelen, "%s+1", nparams[p].prelen);
				} else {
					sprintf(prelen, "sizeof(%s)", nparams[p].type);
				}
				if(get_basetype(nparams[p].type)==0) {
					sprintf(obj_type, "&PROTOCOL_NAME(%s)", nparams[p].type);
				} else {
					sprintf(obj_type, "NULL");
				}
				snprintf(src+strlen(src), src_len-strlen(src), "		{\"%s\", %s, %s, %s, %s, (unsigned int)((char*)(&v_params.%s)-(char*)&v_params)},\n",
					nparams[p].name, stype, obj_type, prelen, "0", nparams[p].name);
				count++;
			}
			snprintf(src+strlen(src), src_len-strlen(src), "	};\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	static PROTOCOL_TYPE v_type = {\"%s.%s\", v_list, %d, sizeof(%s), {\"\", PROTOCOL_TYPE_FAKEVAR|PROTOCOL_TYPE_STRUCT, &v_type, sizeof(\"v_params\"), 0, 0}};\n", nmodules[c].name, nfunctions[f].name, count, "v_params");
			} else {
			snprintf(src+strlen(src), src_len-strlen(src), "	static PROTOCOL_TYPE v_type = {\"%s.%s\", NULL, %d, sizeof(%s), {\"\", PROTOCOL_TYPE_FAKEVAR|PROTOCOL_TYPE_STRUCT, &v_type, sizeof(\"v_params\"), 0, 0}};\n", nmodules[c].name, nfunctions[f].name, count, "v_params");
			}

			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	data_len = nSize;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	if(bTextMode) {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		ret = protocol_text_read(&v_type, \"%s\", (const char*)pData, &data_len, &v_params);\n", nparams[p].name);
			snprintf(src+strlen(src), src_len-strlen(src), "	} else {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		ret = protocol_binary_read(&v_type, pData, &data_len, &v_params);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	if(ret!=ERR_NOERROR)\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		return false;\n");

			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	for(ret=0; ret<nHookCount; ret++) {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		if(pHooks[ret]==NULL) continue;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		pHooks[ret]->%s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p!=nfunctions[f].p_start)
					snprintf(src+strlen(src), src_len-strlen(src), ", ");

				if(get_basetype(nparams[p].type)==0) {
					snprintf(src+strlen(src), src_len-strlen(src), "&v_params.%s", nparams[p].name);
				} else {
					snprintf(src+strlen(src), src_len-strlen(src), "v_params.%s", nparams[p].name);
				}
			}
			snprintf(src+strlen(src), src_len-strlen(src), ");\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	return true;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
		}

		snprintf(src+strlen(src), src_len-strlen(src), "C%sServerBase::C%sServerBase(bool bTextMode, C%sServerHook** pHooks, int nHookMax)\n", nmodules[c].name, nmodules[c].name, nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	m_bTextMode = bTextMode;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	m_pHooks = pHooks;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	m_nHookMax = nHookMax;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "C%sServerBase::~C%sServerBase()\n", nmodules[c].name, nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "bool C%sServerBase::Dispatch(const void* pData, unsigned int nSize)\n", nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	if(m_bTextMode) {\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		const char* end;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		char name[100];\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		end = strchr((const char*)pData, '{');\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		assert(end-(const char*)pData<sizeof(name)-1);\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		if(end-(const char*)pData>sizeof(name)) return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		memcpy(name, pData, end-(const char*)pData);\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		name[end-(const char*)pData] = '\\0';\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		strtrim(strltrim(name));\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='S') continue;
			snprintf(src+strlen(src), src_len-strlen(src), "		if(strcmp(name, \"%s\")==0) {\n", nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "			return %s_%s(true, m_pHooks, m_nHookMax, (const char*)pData, nSize);\n", nmodules[c].name, nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
		}
		snprintf(src+strlen(src), src_len-strlen(src), "		return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	} else {\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		if(nSize==0) return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		switch(*((const unsigned char*)pData)) {\n");
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='S') continue;
			snprintf(src+strlen(src), src_len-strlen(src), "		case %d: // %s\n", f-nmodules[c].f_start, nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "			return %s_%s(false, m_pHooks, m_nHookMax, (const char*)pData+1, nSize-1);\n", nmodules[c].name, nfunctions[f].name);
		}
		snprintf(src+strlen(src), src_len-strlen(src), "		default:\n");
		snprintf(src+strlen(src), src_len-strlen(src), "			return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");

		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='C') continue;
			snprintf(src+strlen(src), src_len-strlen(src), "void C%sServerBase::%s(", nmodules[c].name, nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(src+strlen(src), src_len-strlen(src), ", ");
				snprintf(src+strlen(src), src_len-strlen(src), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
			}
			snprintf(src+strlen(src), src_len-strlen(src), ")\n");
			snprintf(src+strlen(src), src_len-strlen(src), "{\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	int m_nlen;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	unsigned int nSendBufSize;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	char* pSendBuf;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	pSendBuf = GetSendBuf(nSendBufSize);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	if(m_bTextMode) {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen = 0;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s {\" );\n", nfunctions[f].name);
			snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		\n");

			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				switch(get_basetype(nparams[p].type)) {
				case PROTOCOL_TYPE_CHAR:
				case PROTOCOL_TYPE_SHORT:
				case PROTOCOL_TYPE_INT:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%d;\", (int)%s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_LONG:
#ifdef _WIN32
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%I64d;\", %s);\n", nparams[p].name, nparams[p].name);
#else
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%lld;\", %s);\n", nparams[p].name, nparams[p].name);
#endif
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_BYTE:
				case PROTOCOL_TYPE_WORD:
				case PROTOCOL_TYPE_DWORD:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%u;\", (unsigned int)%s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_QWORD:
#ifdef _WIN32
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%I64u;\", %s);\n", nparams[p].name, nparams[p].name);
#else
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%llu;\", %s);\n", nparams[p].name, nparams[p].name);
#endif
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_FLOAT:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=%%f;\", %s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				case PROTOCOL_TYPE_STRING:
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"%s=\\\"%%s\\\";\", %s);\n", nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
					break;
				default:
					snprintf(src+strlen(src), src_len-strlen(src), "		{\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			int ____ret, ____len;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____len = nSendBufSize-m_nlen;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____ret = protocol_text_write(&PROTOCOL_NAME(%s), \"%s=\", %s, pSendBuf+m_nlen, (unsigned int*)&____len);\n", nparams[p].type, nparams[p].name, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "			assert(____ret==ERR_NOERROR);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			if(____ret!=ERR_NOERROR) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			if(m_nlen+(unsigned int)____len>=nSendBufSize) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			pSendBuf[m_nlen+____len] = ';';\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			m_nlen += ____len + 1;\n");

					snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
					break;
				}
			}

			snprintf(src+strlen(src), src_len-strlen(src), "		\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += snprintf(pSendBuf+m_nlen, nSendBufSize-m_nlen, \"}\");\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		assert(m_nlen<(int)nSendBufSize);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		if(m_nlen>=(int)nSendBufSize) return;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen++;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	} else {\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen = 1;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		*((os_byte*)pSendBuf) = %d;\n", f-nmodules[c].f_start);
			snprintf(src+strlen(src), src_len-strlen(src), "\n");

			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start)
					snprintf(src+strlen(src), src_len-strlen(src), "\n");

				switch(get_basetype(nparams[p].type)) {
				case PROTOCOL_TYPE_CHAR:
				case PROTOCOL_TYPE_SHORT:
				case PROTOCOL_TYPE_INT:
				case PROTOCOL_TYPE_LONG:
				case PROTOCOL_TYPE_BYTE:
				case PROTOCOL_TYPE_WORD:
				case PROTOCOL_TYPE_DWORD:
				case PROTOCOL_TYPE_QWORD:
				case PROTOCOL_TYPE_FLOAT:
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(nSendBufSize-m_nlen>=sizeof(%s));\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		if(nSendBufSize-m_nlen<sizeof(%s)) return;\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		*((%s*)(pSendBuf+m_nlen)) = %s;\n", nparams[p].type, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += sizeof(%s);\n", nparams[p].name);
					break;
				case PROTOCOL_TYPE_STRING:
					snprintf(src+strlen(src), src_len-strlen(src), "		assert(nSendBufSize-m_nlen>=sizeof(os_word));\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if(nSendBufSize-m_nlen<sizeof(os_word)) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		*((os_word*)(pSendBuf+m_nlen)) = (os_word)strlen(%s);\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		assert((int)nSendBufSize-m_nlen>=(int)sizeof(os_word)+*((short*)(pSendBuf+m_nlen)));\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		if((int)nSendBufSize-m_nlen<(int)sizeof(os_word)+*((short*)(pSendBuf+m_nlen))) return;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		memcpy(pSendBuf+m_nlen+sizeof(os_word), %s, (size_t)(*((os_word*)(pSendBuf+m_nlen))));\n", nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "		m_nlen += sizeof(os_word) + *((os_word*)(pSendBuf+m_nlen));\n");
					break;
				default:
					snprintf(src+strlen(src), src_len-strlen(src), "		{\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			int ____ret, ____len;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____len = nSendBufSize - m_nlen;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			____ret = protocol_binary_write(&PROTOCOL_NAME(%s), %s, pSendBuf+m_nlen, (unsigned int*)&____len);\n", nparams[p].type, nparams[p].name);
					snprintf(src+strlen(src), src_len-strlen(src), "			assert(____ret==ERR_NOERROR);\n");
					snprintf(src+strlen(src), src_len-strlen(src), "			m_nlen += ____len;\n");
					snprintf(src+strlen(src), src_len-strlen(src), "		}\n");
					break;
				}
			}


			snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	SendBuf(pSendBuf, (unsigned int)m_nlen);\n");
			snprintf(src+strlen(src), src_len-strlen(src), "}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "\n");
		}
	}
	snprintf(src+strlen(src), src_len-strlen(src), "// END OF FILE\n");

	return ERR_NOERROR;
}

const char* parse_include(const char* buf)
{
	const char* tbuf;

	buf = get_token_keyword(buf, "include", name);
	if(buf==NULL) return NULL;
	buf = escape_blank(buf);

	tbuf = buf;
	for(;;) {
		if(*tbuf=='\0') return NULL;
		if(*tbuf<=' ') return NULL;
		if(*tbuf==';') break;
		tbuf++;
	}
	if(tbuf==buf) return NULL;
	if(tbuf-buf>=sizeof(name)) return NULL;
	memcpy(name, buf, tbuf-buf);
	name[tbuf-buf] = '\0';

	buf = get_token_char(tbuf, ';');
	if(buf==NULL) return NULL;

	def_include(name);

	return buf;
}

const char* parse_module(const char* buf)
{
	const char* tbuf;

	buf = get_token_keyword(buf, "module", name);
	if(buf==NULL)
		return NULL;

	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;
	tbuf = buf;
	buf = get_token_char(buf, '{');
	if(buf==NULL) {
		return NULL;
	}

	def_module(name);

	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = parse_function(buf);
		if(tbuf!=NULL) continue;

		//
		return NULL;
	}

	buf = get_token_char(tbuf, ';');
	if(buf==NULL) return NULL;

	return buf;
}

const char* parse_function(const char* buf)
{
	const char* tbuf;

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL)
		return NULL;

	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '(');
	if(buf==NULL) return NULL;

	if(strcmp(type, "client")!=0 && strcmp(type, "server")!=0) {
		printf("invalid function type %s::%s\n", nmodules[nmodules_count-1].name, nfunctions[nfunctions_count-1].name);
		is_break = 1;
		return NULL;
	}

	def_function(type, name);

	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = parse_parameter(buf);
		if(tbuf) {
			// call
			buf = tbuf;
			tbuf = get_token_char(tbuf, ',');
			if(tbuf) continue;
		}

		buf = get_token_char(buf, ')');
		if(buf) break;

		return NULL;
	}

	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	return buf;
}

const char* parse_parameter(const char* buf)
{
	const char* tbuf;
	char slen[100];

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL)
		return NULL;

	tbuf = get_token_char(buf, '<');
	if(tbuf) {
		const char* end;
		end = strchr(tbuf, '>');
		if(!end) return NULL;
		if(end-tbuf>=sizeof(slen)) return NULL;
		memcpy(slen, tbuf, end-tbuf);
		slen[end-tbuf] = '\0';
		buf = end + 1;
	} else {
		slen[0] = '\0';
	}


	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL)
		return NULL;

	tbuf = buf;
	tbuf = get_token_char(buf, '[');
	if(tbuf) {
		tbuf = get_token_number(tbuf, max, sizeof(max));
		if(tbuf==NULL) {
			return NULL;
		}
		tbuf = get_token_char(tbuf, ']');
		if(!tbuf) {
			return NULL;
		}
		buf = tbuf;
	} else {
		max[0] = '\0';
	}
	def_parameter(type, slen, name, max);

	return buf;
}

void def_include(const char* name)
{
	int ret;

	if(!check_include_filename(name)) {
		printf("invalid include filename(%s)\n", name);
		is_break = 1;
		return;
	}

	strcpy(data_include[num_inc].file, name);
	num_inc++;

	ret = parse_file(name);
	if(ret!=ERR_NOERROR) {
		is_break = 1;
	}

	return;
}

void def_module(const char* name)
{
	strcpy(nmodules[nmodules_count].name, name);
	nmodules[nmodules_count].f_start = nfunctions_count;
	nmodules[nmodules_count].f_count = 0;
	nmodules_count++;
}

void def_function(const char* type, const char* name)
{
	nfunctions[nfunctions_count].type = strcmp(type, "client")==0?'C':'S';
	strcpy(nfunctions[nfunctions_count].name, name);
	nfunctions[nfunctions_count].p_start = nparams_count;
	nfunctions[nfunctions_count].p_count = 0;
	nfunctions_count++;
	nmodules[nmodules_count-1].f_count++;
}

void def_parameter(const char* type, const char* prelen, const char* name, const char* max)
{
	strcpy(nparams[nparams_count].type, type);
	strcpy(nparams[nparams_count].prelen, prelen);
	strcpy(nparams[nparams_count].name, name);
	strcpy(nparams[nparams_count].max, max);
	nparams_count++;
	nfunctions[nfunctions_count-1].p_count++;
}
