#include <string.h>
#include <stdlib.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/protocol.h"

struct PROTOCOL_PARSER {
	PROTOCOL_CALLBACK callback;
	int is_break;
};

static const char* get_token_char(const char* buf, char c);
static const char* get_token_string(const char* buf, char* value, int size);
static const char* get_token_id(const char* buf, char* value, int size);
static const char* get_token_keyword(const char* buf, const char* keyword, char* value);
static const char* get_token_number(const char* buf, char* value, int size);
static const char* escape_blank(const char* buf);

static const char* parse_node(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_const(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_field_def(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_array_def(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_field(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_array(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_array_item(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);

int protocol_parse(const char* buf, PROTOCOL_CALLBACK* callback, void* ptr)
{
	while(*buf!='\0') {
		buf = parse_node(callback, ptr, buf);
		if(buf==NULL) return ERR_UNKNOWN;

		buf = escape_blank(buf);
	}

	return ERR_NOERROR;
}

void protocol_break(PROTOCOL_PARSER* p)
{
	p->is_break = 1;
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
	if(end+2<size) return NULL;

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
	buf = get_token_id(buf, id, sizeof(id));
	if(buf==NULL) return NULL;
	if(strcmp(id, keyword)!=0) return NULL;
	if(keyword) strcpy(value, keyword);
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
		if(*buf=='\0') return buf;
		if(*buf<=' ') continue;
		return buf;
	}
}

const char* parse_node(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	const char* tbuf;

	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;
	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	// add node
	if(!callback->new_node_begin(ptr, callback->name)) return NULL;

	// node body
	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = parse_field(callback, ptr, buf);
		if(tbuf!=NULL) continue;

		tbuf = parse_array(callback, ptr, buf);
		if(tbuf!=NULL) continue;

		tbuf = parse_field_def(callback, ptr, buf);
		if(tbuf!=NULL) continue;

		tbuf = parse_array_def(callback, ptr, buf);
		if(tbuf!=NULL) continue;

		//
		return NULL;
	}

	// add node
	if(!callback->new_node_end(ptr)) return NULL;

	return tbuf;
}

const char* parse_const(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	const char* tbuf;

	buf = get_token_keyword(buf, "const", NULL);
	if(buf==NULL) return NULL;

	buf = get_token_id(buf, callback->type, callback->type_len);
	if(buf==NULL) return NULL;
	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;
	tbuf = get_token_string(buf, callback->value, callback->value_len);
	if(tbuf==NULL) {
		tbuf = get_token_id(buf, callback->value, callback->value_len);
		if(tbuf==NULL) {
			tbuf = get_token_number(buf, callback->value, callback->value_len);
			if(tbuf==NULL) {
				return NULL;
			}
		}
	}

	if(!callback->new_const(ptr, callback->type, callback->name, callback->value)) return NULL;

	return tbuf;
}

const char* parse_field_def(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	tbuf = get_token_keyword(buf, "option", callback->mode);
	if(tbuf==NULL) {
		tbuf = get_token_keyword(buf, "acquire", callback->mode);
		if(tbuf==NULL) return NULL;
	}
	buf = tbuf;

	buf = get_token_id(buf, callback->type, callback->type_len);
	if(buf==NULL) return NULL;
	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;

	tbuf = get_token_char(buf, '=');
	if(tbuf!=NULL) {
		buf = tbuf;
		tbuf = get_token_string(buf, callback->value, callback->value_len);
		if(tbuf==NULL) {
			tbuf = get_token_number(buf, callback->value, callback->value_len);
			if(tbuf==NULL) {
				tbuf = get_token_id(buf, callback->value, callback->value_len);
				if(tbuf==NULL) {
					return NULL;
				}
			}
		}
		buf = tbuf;
	} else {
		callback->value[0] = '\0';
	}

	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	if(!callback->new_field_def(ptr, callback->mode, callback->type, callback->name, callback->value)) return NULL;

	return buf;
}

const char* parse_array_def(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	tbuf = get_token_keyword(buf, "option", callback->mode);
	if(tbuf==NULL) {
		tbuf = get_token_keyword(buf, "acquire", callback->mode);
		if(tbuf==NULL) return NULL;
	}
	buf = tbuf;

	buf = get_token_id(buf, callback->type, callback->type_len);
	if(buf==NULL) return NULL;
	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '[');
	if(buf==NULL) return NULL;
	tbuf = get_token_number(buf, callback->value, callback->value_len);
	if(tbuf==NULL) {
		tbuf = get_token_id(buf, callback->value, callback->value_len);
		if(tbuf==NULL) return NULL;
	}
	buf = tbuf;
	buf = get_token_char(buf, ']');
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	if(!callback->new_array_def(ptr, callback->mode, callback->type, callback->name, callback->value)) return NULL;

	return buf;
}

const char* parse_field(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	const char* tbuf;

	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '=');
	if(buf==NULL) return NULL;

	tbuf = get_token_id(buf, callback->value, callback->value_len);
	if(tbuf==NULL) {
		tbuf = get_token_string(buf, callback->value, callback->value_len);
		if(tbuf==NULL) {
			tbuf = get_token_number(buf, callback->value, callback->value_len);
			if(tbuf==NULL) {
				if(!callback->new_field(ptr, callback->name, NULL)) return NULL;
				return parse_array_item(callback, ptr, buf);
			}
		}
	}

	buf = escape_blank(tbuf);
	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	if(!callback->new_field(ptr, callback->name, callback->value)) return NULL;

	return buf;
}

const char* parse_array(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	buf = get_token_id(buf, callback->name, callback->name_len);
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '[');
	if(buf==NULL) return NULL;
	buf = get_token_char(buf, ']');
	if(buf==NULL) return NULL;
	buf = get_token_char(buf, '=');
	if(buf==NULL) return NULL;

	if(!callback->new_array(ptr, callback->name)) return NULL;

	buf = parse_array_item(callback, ptr, buf);
	if(buf==NULL) return NULL;

	return buf;
}

const char* parse_array_item(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	const char* tbuf;

	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	if(!callback->new_begin(ptr)) return NULL;

	tbuf = buf;
	for(;;) {

		if(buf!=tbuf) {
			tbuf = get_token_char(tbuf, ',');
			if(tbuf==NULL) return NULL;
			buf = tbuf;
		}

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = get_token_string(buf, callback->value, callback->value_len);
		if(tbuf==NULL) {
			tbuf = get_token_id(buf, callback->value, callback->value_len);
			if(tbuf==NULL) {
				tbuf = get_token_number(buf, callback->value, callback->value_len);
				if(tbuf==NULL) {
					tbuf = parse_array_item(callback, ptr, buf);
					if(tbuf==NULL) {
						return NULL;
					} else {
						continue;
					}
				}
			}
		}

		if(!callback->new_item(ptr, callback->value)) return NULL;

		return NULL;
	}

	if(!callback->new_end(ptr)) return NULL;

	return tbuf;
}

struct PROTOCOL_VARIABLE {
	char*					name;
	int						type;
	char*					maxlen;
	char*					def_value;
	PROTOCOL_TYPE*			obj_type;
};

struct PROTOCOL_TYPE {
	char*					name;
	PROTOCOL_VARIABLE*		var_list;
	int						var_count;
};

struct PROTOCOL_TABLE {
	int						need_free;
	char*					buf;
	int						buf_count;
	int						buf_max;

	PROTOCOL_TYPE*			type_list;
	int						type_count;
	int						type_max;

	PROTOCOL_VARIABLE*		var_list;
	int						var_count;
	int						var_max;

};

static int my_new_node_begin(void* ptr, const char* name);
static int my_new_node_end(void* ptr);
static int my_new_const(void* ptr, const char* type, const char* name, const char* value);
static int my_new_field_def(void* ptr, const char* mode, const char* type, const char* name, const char* value);
static int my_new_array_def(void* ptr, const char* mode, const char* type, const char* name, const char* count);
static int my_new_field(void* ptr, const char* name, const char* value);
static int my_new_array(void* ptr, const char* name);
static int my_new_begin(void* ptr);
static int my_new_item(void* ptr, const char* value);
static int my_new_end(void* ptr);

static char* protocol_strdup(PROTOCOL_TABLE* table, const char* value);
static PROTOCOL_TYPE* protocol_get_type(PROTOCOL_TABLE* table, const char* name);
static PROTOCOL_VARIABLE* protocol_get_variable(PROTOCOL_TYPE* table, const char* name);
static PROTOCOL_TYPE* protocol_push_type(PROTOCOL_TABLE* table, const char* name);
static PROTOCOL_VARIABLE* protocol_push_variable(PROTOCOL_TABLE* table, char* name, int type, char* maxlen, char* def_value, char* o_type);

PROTOCOL_TABLE* protocol_table_alloc(void* buf, unsigned int buf_len, int type_max, int var_max)
{
	unsigned int size;

	size = sizeof(PROTOCOL_TABLE) + sizeof(PROTOCOL_TYPE)*type_max + sizeof(PROTOCOL_VARIABLE)*var_max;

	if(buf) {
		if(size>buf_len)
			return NULL;
		((PROTOCOL_TABLE*)buf)->need_free = 0;
		((PROTOCOL_TABLE*)buf)->buf = (char*)buf + sizeof(PROTOCOL_TABLE) + sizeof(PROTOCOL_TYPE)*type_max + sizeof(PROTOCOL_VARIABLE)*var_max;
		((PROTOCOL_TABLE*)buf)->buf_count = 0;
		((PROTOCOL_TABLE*)buf)->buf_max = buf_len - sizeof(PROTOCOL_TABLE) - sizeof(PROTOCOL_TYPE)*type_max - sizeof(PROTOCOL_VARIABLE)*var_max;
	} else {
		buf = malloc(size);
		((PROTOCOL_TABLE*)buf)->need_free = 1;
		((PROTOCOL_TABLE*)buf)->buf = 0;
		((PROTOCOL_TABLE*)buf)->buf_count = 0;
		((PROTOCOL_TABLE*)buf)->buf_max = 0;
	}

	((PROTOCOL_TABLE*)buf)->type_list = (PROTOCOL_TYPE*)((char*)buf + sizeof(PROTOCOL_TABLE));
	((PROTOCOL_TABLE*)buf)->type_count = 0;
	((PROTOCOL_TABLE*)buf)->type_max = type_max;
	memset(((PROTOCOL_TABLE*)buf)->type_list, 0, sizeof(PROTOCOL_TYPE)*type_max);

	((PROTOCOL_TABLE*)buf)->var_list = (PROTOCOL_VARIABLE*)((char*)buf + sizeof(PROTOCOL_TABLE) + sizeof(PROTOCOL_TYPE)*type_max);
	((PROTOCOL_TABLE*)buf)->var_count = 0;
	((PROTOCOL_TABLE*)buf)->var_max = var_max;
	memset(((PROTOCOL_TABLE*)buf)->var_list, 0, sizeof(PROTOCOL_VARIABLE)*var_max);

	return (PROTOCOL_TABLE*)buf;
}

void protocol_table_free(PROTOCOL_TABLE* table)
{
	if(table->need_free) {
		protocol_table_clear(table);
		free(table);
	}
}

void protocol_table_clear(PROTOCOL_TABLE* table)
{
	if(table->need_free) {
		int i;
		for(i=0; i<table->type_count; i++) {
			if(table->type_list[i].name)
				free(table->type_list[i].name);
		}
		for(i=0; i<table->var_count; i++) {
			if(table->var_list[i].name)
				free(table->var_list[i].name);
			if(table->var_list[i].def_value)
				free(table->var_list[i].def_value);
		}
	}
	memset(table->type_list, 0, sizeof(table->type_list[0])*table->type_max);
	memset(table->var_list, 0, sizeof(table->var_list[0])*table->var_max);
	table->type_count = 0;
	table->var_count = 0;
}

int protocol_parse_pfile(const char* text, PROTOCOL_TABLE* table)
{
	int ret;
	char mode[100], type[100], name[100], value[2000];
	PROTOCOL_CALLBACK callback = {
	my_new_node_begin, my_new_node_end, my_new_const, my_new_field_def, my_new_array_def,
	my_new_field, my_new_array, my_new_begin, my_new_item, my_new_end,
	mode, type, name, value,
	sizeof(mode), sizeof(type), sizeof(name), sizeof(value)
	};

	ret = protocol_parse(text, &callback, table);

	return ret;
}

int protocol_generate_cfile(const PROTOCOL_TABLE* table, const char* name, char* inc, unsigned int inc_len, char* src, unsigned int src_len)
{
	return ERR_NOERROR;
}

int protocol_binary_read(PROTOCOL_TABLE* table, const char* name, const void* data, unsigned int data_len, void* buf)
{
	return ERR_NOERROR;
}

int protocol_binary_write(PROTOCOL_TABLE* table, const char* name, const void* buf, void* data, unsigned int* data_len)
{
	return ERR_NOERROR;
}

int protocol_text_read(PROTOCOL_TABLE* table, const char* name, const char* data, void* buf)
{
	return ERR_NOERROR;
}

int protocol_text_write(PROTOCOL_TABLE* table, const char* name, const void* buf, char* data, unsigned int data_len)
{
	return ERR_NOERROR;
}

int protocol_file_read(PROTOCOL_TABLE* table, const char* name, const char* filename, void* buf)
{
	return ERR_NOERROR;
}

int protocol_file_write(PROTOCOL_TABLE* table, const char* name, const void* buf, const char* filename)
{
	return ERR_NOERROR;
}

char* protocol_strdup(PROTOCOL_TABLE* table, const char* value)
{
	char* ret;
	if(table->need_free) {
		ret = malloc(strlen(value)+1);
		strcpy(ret, value);
	} else {
		if(table->buf_count+(int)strlen(value)+1 > table->buf_max) {
			return NULL;
		}
		ret = table->buf + table->buf_count;
		table->buf_count += strlen(value) + 1;
		strcpy(ret, value);
	}
	return ret;
}

PROTOCOL_TYPE* protocol_get_type(PROTOCOL_TABLE* table, const char* name)
{
	int i;
	for(i=0; i<table->type_count; i++) {
		if(strcmp(table->type_list[i].name, name)==0) {
			return &table->type_list[i];
		}
	}
	return NULL;
}

PROTOCOL_VARIABLE* protocol_get_variable(PROTOCOL_TYPE* table, const char* name)
{
	int i;
	for(i=0; i<table->var_count; i++) {
		if(strcmp(table->var_list[i].name, name)==0) {
			return &table->var_list[i];
		}
	}
	return NULL;
}

PROTOCOL_TYPE* protocol_push_type(PROTOCOL_TABLE* table, const char* name)
{
	if(table->type_count==table->type_max)
		return NULL;

	if(name==NULL)
		return NULL;

	table->type_list[table->type_count].name = protocol_strdup(table, name);
	if(!table->type_list[table->type_count].name)
		return NULL;

	table->type_list[table->type_count].var_list = &table->var_list[table->var_count];
	table->type_list[table->type_count].var_count = 0;

	return &table->type_list[table->type_count++];
}

PROTOCOL_VARIABLE* protocol_push_variable(PROTOCOL_TABLE* table, char* name, int type, char* maxlen, char* def_value, char* o_type)
{
	if(table->var_count==table->var_max)
		return NULL;

	if(name==NULL)
		return NULL;

	table->var_list[table->var_count].name = protocol_strdup(table, name);
	if(!table->var_list[table->var_count].name)
		return NULL;

	if(def_value) {
		table->var_list[table->var_count].def_value = protocol_strdup(table, def_value);
		if(!table->var_list[table->var_count].def_value)
			return NULL;
	} else {
		table->var_list[table->var_count].def_value = NULL;
	}

	return &table->var_list[table->var_count++];
}

int my_new_node_begin(void* ptr, const char* name)
{
	return 1;
}

int my_new_node_end(void* ptr)
{
	return 1;
}

int my_new_const(void* ptr, const char* type, const char* name, const char* value)
{
	return 1;
}

int my_new_field_def(void* ptr, const char* mode, const char* type, const char* name, const char* value)
{
	return 1;
}

int my_new_array_def(void* ptr, const char* mode, const char* type, const char* name, const char* count)
{
	return 1;
}

int my_new_field(void* ptr, const char* name, const char* value)
{
	return 0;
}

int my_new_array(void* ptr, const char* name)
{
	return 0;
}

int my_new_begin(void* ptr)
{
	return 0;
}

int my_new_item(void* ptr, const char* value)
{
	return 0;
}

int my_new_end(void* ptr)
{
	return 0;
}
