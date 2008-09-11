#include <string.h>

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