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

	ret = parser_nfile(txt);
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
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "include %s\n", data_include[i].file);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	for(c=0; c<nmodules_count; c++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "interface I%sClient {\n", nmodules[c].name);
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='C') continue;
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	void %s(", nfunctions[f].name);
			for(p=nfunctions[f].p_start; p<nfunctions[f].p_start+nfunctions[f].p_count; p++) {
				if(p>nfunctions[f].p_start) snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
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
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(nparams[p].type), nparams[p].name);
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ");\n");
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	}

	return ERR_NOERROR;
}

int generate_cfile(const char* name, char* src, unsigned int src_len)
{
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
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class C%sClient : public I%sClient {\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "protected:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sClient() { m_bHandled = false; }\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	~C%sClient() {}\n", nmodules[c].name);
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
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class C%sClientInstance {\n", nmodules[c].name, nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	C%sClientInstance();\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	~C%sClientInstance();\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	bool Dispatch(const void* pData, unsigned int nSize);\n", nmodules[c].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual C%sClient* GetNext(C%sClient* pClient);\n", nmodules[c].name, nmodules[c].name);

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
	int c, f;

	src[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	src[0] = '\0';
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// generate by NPROT_GEN.\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// %s\n", asctime(newTime));
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.clt.hpp\"\n", name);
	snprintf(src+strlen(src), src_len-strlen(src), "\n");

	for(c=0; c<nmodules_count; c++) {
		for(f=nmodules[c].f_start; f<nmodules[c].f_start+nmodules[c].f_count; f++) {
			if(nfunctions[f].type!='S') continue;
		}

		snprintf(src+strlen(src), src_len-strlen(src), "C%sClientInstance::C%sClientInstance()\n", nmodules[c].name, nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "C%sClientInstance::C%sClientInstance()\n", nmodules[c].name, nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "bool C%sClientInstance::Dispatch(const void* pData, unsigned int nSize)\n", nmodules[c].name);
		snprintf(src+strlen(src), src_len-strlen(src), "{\n");
		snprintf(src+strlen(src), src_len-strlen(src), "	return false;\n");
		snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
	}
	snprintf(src+strlen(src), src_len-strlen(src), "// END OF FILE\n");

	return ERR_NOERROR;
}

int generate_hsvrfile(const char* name, char* inc, unsigned int inc_len)
{
	return ERR_NOERROR;
}

int generate_csvrfile(const char* name, char* src, unsigned int src_len)
{
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

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL)
		return NULL;

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
	def_parameter(type, "", name, max);

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
