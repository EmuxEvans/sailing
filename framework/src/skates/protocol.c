#include <string.h>
#include <stdlib.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/misc.h"
#include "../../inc/skates/protocol.h"

static const char* get_token_char(const char* buf, char c);
static const char* get_token_string(const char* buf, char* value, int size);
static const char* get_token_id(const char* buf, char* value, int size);
static const char* get_token_keyword(const char* buf, const char* keyword, char* value);
static const char* get_token_number(const char* buf, char* value, int size);
static const char* escape_blank(const char* buf);

static const char* parse_node(PROTOCOL_CALLBACK* callback, const char* buf);
static const char* parse_field(PROTOCOL_CALLBACK* callback, const char* buf);
static const char* parse_array(PROTOCOL_CALLBACK* callback, const char* buf);
static const char* parse_array_item(PROTOCOL_CALLBACK* callback, const char* buf);

static void proto_new_field(PROTOCOL_CALLBACK* callback, const char* name, const char* value);
static void proto_new_array(PROTOCOL_CALLBACK* callback, const char* name);
static void proto_new_begin(PROTOCOL_CALLBACK* callback);
static void proto_new_item(PROTOCOL_CALLBACK* callback, const char* value);
static void proto_new_end(PROTOCOL_CALLBACK* callback);
static int proto_convert(const char* value, void* buf, int type, unsigned int len);

typedef struct PROTO_PARSE {
	PROTOCOL_TYPE*			root;
	void*					buf;
	struct {
		int					type;
		PROTOCOL_TYPE*		obj_type;
		void*				buf;
		unsigned int		prelen;
		unsigned int		max;
		unsigned int		count;
	} stack[10];
	int stack_count;
} PROTO_PARSE;

int protocol_parse(const char* buf, PROTOCOL_CALLBACK* callback, void* ptr)
{
	callback->is_break = 0;
	callback->user_ptr = ptr;

	buf = get_token_id(buf, callback->name, callback->name_len);
	if(!buf) return ERR_UNKNOWN;

	callback->new_field(callback, callback->name, NULL);
	if(callback->is_break) return ERR_UNKNOWN;

	buf = parse_node(callback, buf);
	if(callback->is_break) return ERR_UNKNOWN;
	if(!buf) return ERR_UNKNOWN;

	return ERR_NOERROR;
}

void protocol_break(PROTOCOL_CALLBACK* callback)
{
	(void)get_token_keyword;
	callback->is_break = 1;
}

const char* get_token_char(const char* buf, char c)
{
	buf = escape_blank(buf);
	if(*buf!=c) return NULL;
	return buf+1;
}

const char* get_token_string(const char* buf, char* value, int size)
{
	int end;

	buf = escape_blank(buf);
	if(*buf!='"') return NULL;
	for(end=1; ;end++) {
		if(buf[end]=='\0') return NULL;
		if(buf[end]=='"') break;
		if(buf[end]=='\\') {
			if(buf[end-1]=='\0') return NULL;
			end++; continue;
		}
	}
	if(end+2>size) return NULL;

	memcpy(value, buf, end+1);
	value[end+1] = '\0';

	return buf+end+1;
}

const char* get_token_id(const char* buf, char* value, int size)
{
	int end;

	buf = escape_blank(buf);

	for(end=0;; end++) {
		if(buf[end]>='0' && buf[end]<='9') {
			if(end==0) return NULL;
			continue;
		}
		if(buf[end]>='a' && buf[end]<='z') continue;
		if(buf[end]>='A' && buf[end]<='Z') continue;
		if(buf[end]=='_') continue;

		break;
	}
	if(end==0) return NULL;

	if(end+1>size) return NULL;
	memcpy(value, buf, end);
	value[end] = '\0';
	return buf+end;
}

const char* get_token_keyword(const char* buf, const char* keyword, char* value)
{
	char id[100];
	buf = escape_blank(buf);
	buf = get_token_id(buf, id, sizeof(id));
	if(buf==NULL) return NULL;
	if(strcmp(id, keyword)!=0) return NULL;
	if(value) strcpy(value, keyword);
	return buf;
}

const char* get_token_number(const char* buf, char* value, int size)
{
	int end;

	buf = escape_blank(buf);

	for(end=0; ;end++) {
		if(buf[end]>='0' && buf[end]<='9') continue;
		if(buf[end]=='+') continue;
		if(buf[end]=='-') continue;
		if(buf[end]=='.') continue;
		if(end==0) return NULL;
		break;
	}

	if(end+1>size) return NULL;
	memcpy(value, buf, end);
	value[end] = '\0';

	return buf+end;
}

const char* escape_blank(const char* buf)
{
	for(;;buf++) {

		if(*buf=='/' && buf[1]=='/') {
			buf+=2;
			while(*buf!='\n' && *buf!='\0') buf++;
		}
		if(*buf=='\0') return buf;
		if(*buf<=' ') continue;

		return buf;
	}
}

const char* parse_node(PROTOCOL_CALLBACK* callback, const char* buf)
{
	const char* tbuf;

	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	// add node
	callback->new_begin(callback);
	if(callback->is_break) return NULL;

	// node body
	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = parse_field(callback, buf);
		if(callback->is_break) return NULL;
		if(tbuf!=NULL) continue;

		tbuf = parse_array(callback, buf);
		if(callback->is_break) return NULL;
		if(tbuf!=NULL) continue;

		//
		return NULL;
	}

	// add node
	callback->new_end(callback);

	return tbuf;
}

const char* parse_field(PROTOCOL_CALLBACK* callback, const char* buf)
{
	const char* tbuf;

	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '=');
	if(buf==NULL) return NULL;

	tbuf = get_token_char(buf, '{');
	if(tbuf) {
		callback->new_field(callback, callback->name, NULL);
		tbuf = parse_node(callback, buf);
		if(!tbuf) return NULL;
	} else {
		tbuf = get_token_id(buf, callback->value, callback->value_len);
		if(tbuf==NULL) {
			tbuf = get_token_string(buf, callback->value, callback->value_len);
			if(tbuf==NULL) {
				tbuf = get_token_number(buf, callback->value, callback->value_len);
				if(tbuf==NULL) {
					return NULL;
				}
			}
		}
		callback->new_field(callback, callback->name, callback->value);
	}

	buf = get_token_char(tbuf, ';');
	if(buf==NULL) return NULL;

	return buf;
}

const char* parse_array(PROTOCOL_CALLBACK* callback, const char* buf)
{
	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '[');
	if(buf==NULL) return NULL;
	buf = get_token_char(buf, ']');
	if(buf==NULL) return NULL;
	buf = get_token_char(buf, '=');
	if(buf==NULL) return NULL;

	callback->new_array(callback, callback->name);
	if(callback->is_break) return NULL;

	buf = parse_array_item(callback, buf);
	if(callback->is_break) return NULL;
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, ';');
	if(callback->is_break) return NULL;
	if(buf==NULL) return NULL;

	return buf;
}

const char* parse_array_item(PROTOCOL_CALLBACK* callback, const char* buf)
{
	const char* tbuf;

	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	callback->new_begin(callback);
	if(callback->is_break) return NULL;

	tbuf = buf;
	for(;;) {

		if(buf!=tbuf) {
			tbuf = get_token_char(tbuf, ',');
			if(tbuf==NULL) return NULL;
			buf = tbuf;
		}

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = get_token_char(buf, '{');
		if(tbuf) {
			callback->new_item(callback, NULL);
			tbuf = parse_node(callback, buf);
			if(!tbuf) return NULL;
		} else {
			tbuf = get_token_string(buf, callback->value, callback->value_len);
			if(tbuf==NULL) {
				tbuf = get_token_id(buf, callback->value, callback->value_len);
				if(tbuf==NULL) {
					tbuf = get_token_number(buf, callback->value, callback->value_len);
					if(tbuf==NULL) {
						tbuf = parse_array_item(callback, buf);
						if(!tbuf)
							return NULL;
					}
				}
			}
			callback->new_item(callback, callback->value);
			if(callback->is_break) return NULL;
		}
	}

	callback->new_end(callback);

	return tbuf;
}

#define READ_BUFFER(addr, size)	\
	if(read_len+size>*data_len) return ERR_UNKNOWN;	\
	memcpy(addr, (const char*)data+read_len, size);		\
	read_len += size;

#define WRITE_BUFFER(addr, size)	\
	if(write_len+size>*data_len) return ERR_UNKNOWN;	\
	memcpy((char*)data+write_len, addr, size);		\
	write_len += size;

int protocol_binary_read(PROTOCOL_TYPE* type, const void* data, unsigned int* data_len, void* buf)
{
	int i, j, ret;
	unsigned int read_len = 0;
	unsigned int len;

	for(i=0; i<type->var_count; i++) {
		if(type->var_list[i].type&PROTOCOL_TYPE_ARRAY) {
			READ_BUFFER(&len, sizeof(len));
			if(len>type->var_list[i].maxlen)
				return ERR_INVALID_DATA;
			*((unsigned int*)((char*)buf+type->var_list[i].offset-sizeof(unsigned int))) = len;
		} else {
			len = 1;
		}

		for(j=0; j<(int)len; j++) {
			if((type->var_list[i].type&0xff)==PROTOCOL_TYPE_OBJECT) {
				unsigned int olen;
				olen = *data_len - read_len;
				ret = protocol_binary_read(type->var_list[i].obj_type, (const char*)data+read_len, &olen, (char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen);
				if(ret!=ERR_NOERROR) return ret;
				read_len += olen;
				continue;
			}
			if((type->var_list[i].type&0xff)==PROTOCOL_TYPE_STRING) {
				unsigned int slen;
				slen = strlen((const char*)data+read_len)+1;
				if(slen>type->var_list[i].prelen) slen = type->var_list[i].prelen;
				READ_BUFFER((char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen, slen);
				continue;
			}
			READ_BUFFER((char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen, type->var_list[i].prelen);
		}
	}

	*data_len = read_len;

	return ERR_NOERROR;
}

int protocol_binary_write(PROTOCOL_TYPE* type, const void* buf, void* data, unsigned int* data_len)
{
	int i, j, ret;
	unsigned int write_len = 0;
	unsigned int len;

	for(i=0; i<type->var_count; i++) {
		if(type->var_list[i].type&PROTOCOL_TYPE_ARRAY) {
			len = *((const unsigned int*)((const char*)buf+type->var_list[i].offset-sizeof(unsigned int)));
			if(len>type->var_list[i].maxlen)
				return ERR_INVALID_DATA;
			WRITE_BUFFER(&len, sizeof(len));
		} else {
			len = 1;
		}

		for(j=0; j<(int)len; j++) {
			if((type->var_list[i].type&0xff)==PROTOCOL_TYPE_OBJECT) {
				unsigned int olen;
				olen = *data_len - write_len;
				ret = protocol_binary_write(type->var_list[i].obj_type, (const char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen, (char*)data+write_len, &olen);
				if(ret!=ERR_NOERROR) return ret;
				write_len += olen;
				continue;
			}
			if((type->var_list[i].type&0xff)==PROTOCOL_TYPE_STRING) {
				unsigned int slen;
				slen = strlen((const char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen)+1;
				if(slen>type->var_list[i].prelen) slen = type->var_list[i].prelen;
				WRITE_BUFFER((const char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen, slen);
				continue;
			}
			WRITE_BUFFER((const char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen, type->var_list[i].prelen);
		}
	}

	*data_len = write_len;

	return ERR_NOERROR;
}

#define WRITE_TEXT(...)	\
	ret = (int)snprintf(data+write_len, *data_len-write_len, __VA_ARGS__);	\
	if(ret>=(int)(*data_len-write_len)) return ERR_INVALID_DATA;	\
	write_len += (unsigned int)ret;

int protocol_text_read(PROTOCOL_TYPE* type, const char* data, void* buf)
{
	PROTO_PARSE parse;
	char callback_type[100];
	char callback_name[100];
	char callback_value[100];
	PROTOCOL_CALLBACK callback = {
		proto_new_field,
		proto_new_array,
		proto_new_begin,
		proto_new_item,
		proto_new_end,

		callback_type,
		callback_name,
		callback_value,
		sizeof(callback_type),
		sizeof(callback_name),
		sizeof(callback_value),

		0
	};

	parse.root = type;
	parse.buf = buf;
	parse.stack_count = 0;

	return protocol_parse(data, &callback, &parse);
}

int text_write_object(PROTOCOL_TYPE* type, const void* buf, char* data, unsigned int* data_len)
{
	int ret, i, j;
	unsigned int write_len = 0, len, count;
	WRITE_TEXT("{");
	for(i=0; i<type->var_count; i++) {
		if(type->var_list[i].type&PROTOCOL_TYPE_ARRAY) {
			WRITE_TEXT("%s[]=", type->var_list[i].name);
			count = *((unsigned int*)((const char*)buf+type->var_list[i].offset-4));
			if(count>type->var_list[i].maxlen) return ERR_INVALID_DATA;
		} else {
			WRITE_TEXT("%s=", type->var_list[i].name);
			count = 1;
		}

		if(type->var_list[i].type&PROTOCOL_TYPE_ARRAY) {
			WRITE_TEXT("{");
		}
		for(j=0; j<(int)count; j++) {
			if(j>0) {
				WRITE_TEXT(",");
			}

			switch(type->var_list[i].type&0xff) {
			case PROTOCOL_TYPE_CHAR:
				WRITE_TEXT("%d", (int)(((const os_char*)((char*)buf+type->var_list[i].offset))[j]));
				break;
			case PROTOCOL_TYPE_SHORT:
				WRITE_TEXT("%d", (int)(((const os_short*)((char*)buf+type->var_list[i].offset))[j]));
				break;
			case PROTOCOL_TYPE_INT:
				WRITE_TEXT("%d", (int)(((const os_int*)((char*)buf+type->var_list[i].offset))[j]));
				break;
			case PROTOCOL_TYPE_LONG:
#ifdef _WIN32
				WRITE_TEXT("%I64d", ((const os_long*)((char*)buf+type->var_list[i].offset))[j]);
#else
				WRITE_TEXT("%lld", ((const os_long*)((char*)buf+type->var_list[i].offset))[j]);
#endif
				break;
			case PROTOCOL_TYPE_BYTE:
				WRITE_TEXT("%u", (unsigned int)(((const os_byte*)((char*)buf+type->var_list[i].offset))[j]));
				break;
			case PROTOCOL_TYPE_WORD:
				WRITE_TEXT("%u", (unsigned int)(((const os_word*)((char*)buf+type->var_list[i].offset))[j]));
				break;
			case PROTOCOL_TYPE_DWORD:
				WRITE_TEXT("%u", (unsigned int)(((const os_dword*)((char*)buf+type->var_list[i].offset))[j]));
				break;
			case PROTOCOL_TYPE_QWORD:
#ifdef _WIN32
				WRITE_TEXT("%I64u", ((const os_qword*)((char*)buf+type->var_list[i].offset))[j]);
#else
				WRITE_TEXT("%llu", ((const os_qword*)((char*)buf+type->var_list[i].offset))[j]);
#endif
				break;
			case PROTOCOL_TYPE_FLOAT:
				WRITE_TEXT("%f", (double)(((const os_qword*)((char*)buf+type->var_list[i].offset))[j]));
				break;
			case PROTOCOL_TYPE_STRING:
				WRITE_TEXT("\"%s\"", (char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen);
				break;
			case PROTOCOL_TYPE_OBJECT:
				len = *data_len - write_len;
				ret = text_write_object(type->var_list[i].obj_type, (const char*)buf+type->var_list[i].offset+j*type->var_list[i].prelen, (char*)data+write_len, &len);
				if(ret!=ERR_NOERROR) return ret;
				write_len += len;
				break;
			}
		}
		if(type->var_list[i].type&PROTOCOL_TYPE_ARRAY) {
			WRITE_TEXT("}");
		}
		WRITE_TEXT(";");
	}
	WRITE_TEXT("}");
	*data_len = write_len;
	return ERR_NOERROR;
}

int protocol_text_write(PROTOCOL_TYPE* type, const void* buf, char* data, unsigned int* data_len)
{
	int ret;
	unsigned int write_len = 0, len;
	WRITE_TEXT("%s ", type->name);
	len = *data_len - write_len - 1;
	ret = text_write_object(type, buf, data+write_len, &len);
	if(ret!=ERR_NOERROR) return ret;
	write_len += len;
	data[write_len] = '\0';
	*data_len = write_len + 1;
	return ERR_NOERROR;
}

int protocol_file_read(PROTOCOL_TYPE* type, const char* filename, void* buf)
{
	int ret;
	char data[50*1024];

	ret = load_textfile(filename, data, sizeof(data));
	if(ret<0)
		return ret;

	return protocol_text_read(type, data, buf);
}

int protocol_file_write(PROTOCOL_TYPE* type, const void* buf, const char* filename)
{
	int ret;
	char data[50*1024];
	unsigned data_len = sizeof(data);

	ret = protocol_text_write(type, buf, data, &data_len);
	if(ret!=ERR_NOERROR)
		return ret;

	ret = save_textfile(filename, data, sizeof(data));
	return ret<0?ERR_UNKNOWN:ERR_NOERROR;
}

void proto_new_field(PROTOCOL_CALLBACK* callback, const char* name, const char* value)
{
	int i;
	PROTO_PARSE* parse = (PROTO_PARSE*)callback->user_ptr;
	if(parse->stack_count==0) {
		parse->stack[0].type = PROTOCOL_TYPE_OBJECT;
		parse->stack[0].obj_type = parse->root;
		parse->stack[0].prelen = parse->root->size;
		parse->stack[0].buf = parse->buf;
		parse->stack[0].max = 0;
		parse->stack[0].count = 0;
		return;
	}

	for(i=0; i<parse->stack[parse->stack_count-1].obj_type->var_count; i++) {
		if(strcmp(parse->stack[parse->stack_count-1].obj_type->var_list[i].name, name)==0) {
			break;
		}
	}
	if(i==parse->stack[parse->stack_count-1].obj_type->var_count) {
		protocol_break(callback);
		return;
	}

	if(!value) {
		if(parse->stack[parse->stack_count-1].obj_type->var_list[i].type!=PROTOCOL_TYPE_OBJECT) {
			protocol_break(callback);
			return;
		}
		parse->stack[parse->stack_count].type = parse->stack[parse->stack_count-1].obj_type->var_list[i].type;
		parse->stack[parse->stack_count].obj_type = parse->stack[parse->stack_count-1].obj_type->var_list[i].obj_type;
		parse->stack[parse->stack_count].prelen = parse->stack[parse->stack_count-1].obj_type->var_list[i].prelen;
		parse->stack[parse->stack_count].buf = (char*)parse->stack[parse->stack_count-1].buf + parse->stack[parse->stack_count-1].obj_type->var_list[i].offset; 
		parse->stack[parse->stack_count].max = 0;
		parse->stack[parse->stack_count].count = 0;
		return;
	}

	switch(parse->stack[parse->stack_count-1].obj_type->var_list[i].type) {
	case PROTOCOL_TYPE_STRING:
	case PROTOCOL_TYPE_CHAR:
	case PROTOCOL_TYPE_SHORT:
	case PROTOCOL_TYPE_INT:
	case PROTOCOL_TYPE_LONG:
	case PROTOCOL_TYPE_BYTE:
	case PROTOCOL_TYPE_WORD:
	case PROTOCOL_TYPE_DWORD:
	case PROTOCOL_TYPE_QWORD:
	case PROTOCOL_TYPE_FLOAT:
		break;
	default:
		protocol_break(callback);
		return;
	}

	if(proto_convert(value, (char*)parse->stack[parse->stack_count-1].buf + parse->stack[parse->stack_count-1].obj_type->var_list[i].offset, parse->stack[parse->stack_count-1].obj_type->var_list[i].type, parse->stack[parse->stack_count-1].obj_type->var_list[i].prelen)!=ERR_NOERROR) {
		protocol_break(callback);
		return;
	}
//	memset((char*)parse->stack[parse->stack_count-1].buf + parse->stack[parse->stack_count-1].obj_type->var_list[i].offset, 0x33, parse->stack[parse->stack_count-1].obj_type->var_list[i].prelen);
}

void proto_new_array(PROTOCOL_CALLBACK* callback, const char* name)
{
	int i;
	PROTO_PARSE* parse = (PROTO_PARSE*)callback->user_ptr;

	for(i=0; i<parse->stack[parse->stack_count-1].obj_type->var_count; i++) {
		if(strcmp(parse->stack[parse->stack_count-1].obj_type->var_list[i].name, name)==0) {
			break;
		}
	}
	if(i==parse->stack[parse->stack_count-1].obj_type->var_count) {
		protocol_break(callback);
		return;
	}

	if((parse->stack[parse->stack_count-1].obj_type->var_list[i].type&PROTOCOL_TYPE_ARRAY)==0) {
		protocol_break(callback);
		return;
	}

	parse->stack[parse->stack_count].type = parse->stack[parse->stack_count-1].obj_type->var_list[i].type;
	parse->stack[parse->stack_count].obj_type = parse->stack[parse->stack_count-1].obj_type->var_list[i].obj_type;
	parse->stack[parse->stack_count].prelen = parse->stack[parse->stack_count-1].obj_type->var_list[i].prelen;
	parse->stack[parse->stack_count].buf = (char*)parse->stack[parse->stack_count-1].buf + parse->stack[parse->stack_count-1].obj_type->var_list[i].offset;
	parse->stack[parse->stack_count].max = 0;
	parse->stack[parse->stack_count].count = 0;
}

void proto_new_begin(PROTOCOL_CALLBACK* callback)
{
	PROTO_PARSE* parse = (PROTO_PARSE*)callback->user_ptr;
	parse->stack_count++;
}

void proto_new_item(PROTOCOL_CALLBACK* callback, const char* value)
{
	PROTO_PARSE* parse = (PROTO_PARSE*)callback->user_ptr;

	if(parse->stack[parse->stack_count-1].type==(PROTOCOL_TYPE_ARRAY|PROTOCOL_TYPE_OBJECT)) {
		if(value) {
			protocol_break(callback);
			return;
		}
		if(parse->stack[parse->stack_count-1].count>=parse->stack[parse->stack_count-1].max) {
			protocol_break(callback);
			return;
		}

		parse->stack[parse->stack_count].type = parse->stack[parse->stack_count-1].type & 0xff;
		parse->stack[parse->stack_count].obj_type = parse->stack[parse->stack_count-1].obj_type;
		parse->stack[parse->stack_count].prelen = parse->stack[parse->stack_count-1].prelen;
		parse->stack[parse->stack_count].buf = (char*)parse->stack[parse->stack_count-1].buf + parse->stack[parse->stack_count-1].obj_type->size * parse->stack[parse->stack_count-1].count;
		parse->stack[parse->stack_count].max = 0;
		parse->stack[parse->stack_count].count = 0;
		parse->stack[parse->stack_count-1].count++;
		return;
	}

	if(!value) {
		protocol_break(callback);
		return;
	}

	if(proto_convert(value, (char*)parse->stack[parse->stack_count-1].buf + parse->stack[parse->stack_count-1].prelen * parse->stack[parse->stack_count-1].count, parse->stack[parse->stack_count-1].type, parse->stack[parse->stack_count-1].prelen)!=ERR_NOERROR) {
		protocol_break(callback);
		return;
	}
	//memset(
	//	(char*)parse->stack[parse->stack_count-1].buf + parse->stack[parse->stack_count-1].prelen * parse->stack[parse->stack_count-1].count,
	//	0x33, parse->stack[parse->stack_count-1].prelen);
}

void proto_new_end(PROTOCOL_CALLBACK* callback)
{
	PROTO_PARSE* parse = (PROTO_PARSE*)callback->user_ptr;
	parse->stack_count--;

	if(parse->stack[parse->stack_count].type&PROTOCOL_TYPE_ARRAY) {
		*((unsigned int*)((char*)parse->stack[parse->stack_count].buf - 4)) = parse->stack[parse->stack_count].count;
	}
}

int proto_convert(const char* value, void* buf, int type, unsigned int len)
{
	os_long v;

	if(type==PROTOCOL_TYPE_STRING) {
		unsigned int slen;
		if(value[0]!='"') return ERR_UNKNOWN;
		slen = strlen(value);
		if(slen-1>len) return ERR_UNKNOWN;
		memcpy(buf, value+1, slen-2);
		*((char*)buf+slen-2) = '\0';
		return ERR_NOERROR;
	}
	if(type==PROTOCOL_TYPE_FLOAT) {
		if(sscanf(value, "%f", (os_float*)buf)!=1) return ERR_UNKNOWN;
		return ERR_NOERROR;
	}

#ifdef _WIN32
	v = _atoi64(value);
#else
	v = atoll(value);
#endif

	switch(type) {
	case PROTOCOL_TYPE_CHAR:
		*((os_char*)buf) = (os_char)v;
		break;
	case PROTOCOL_TYPE_SHORT:
		*((os_short*)buf) = (os_short)v;
		break;
	case PROTOCOL_TYPE_INT:
		*((os_int*)buf) = (os_int)v;
		break;
	case PROTOCOL_TYPE_LONG:
		*((os_long*)buf) = (os_long)v;
		break;
	case PROTOCOL_TYPE_BYTE:
		*((os_byte*)buf) = (os_byte)v;
		break;
	case PROTOCOL_TYPE_WORD:
		*((os_word*)buf) = (os_word)v;
		break;
	case PROTOCOL_TYPE_DWORD:
		*((os_dword*)buf) = (os_dword)v;
		break;
	case PROTOCOL_TYPE_QWORD:
		*((os_qword*)buf) = (os_qword)v;
		break;
	default:
		return ERR_UNKNOWN;
	}
	return ERR_NOERROR;
}
