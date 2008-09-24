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

static const char* parse_node(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_field(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_array(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);
static const char* parse_array_item(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf);

int protocol_parse(const char* buf, PROTOCOL_CALLBACK* callback, void* ptr)
{
	const char* tbuf;

	callback->is_break = 0;
	tbuf = buf;
	for(;;) {
		buf = escape_blank(tbuf);
		if(*buf=='\0') break;

		tbuf = parse_node(callback, ptr, buf);
		if(tbuf) continue;

		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
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
	callback->new_node_begin(ptr, callback->name);

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

		//
		return NULL;
	}

	// add node
	callback->new_node_end(ptr);

	return tbuf;
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
				callback->new_field(ptr, callback->name, NULL);
				return parse_array_item(callback, ptr, buf);
			}
		}
	}

	buf = escape_blank(tbuf);
	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	callback->new_field(ptr, callback->name, callback->value);

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

	callback->new_array(ptr, callback->name);

	buf = parse_array_item(callback, ptr, buf);
	if(buf==NULL) return NULL;

	return buf;
}

const char* parse_array_item(PROTOCOL_CALLBACK* callback, void* ptr, const char* buf)
{
	const char* tbuf;

	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	callback->new_begin(ptr);

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

		callback->new_item(ptr, callback->value);

		return NULL;
	}

	callback->new_end(ptr);

	return tbuf;
}

int protocol_binary_read(PROTOCOL_TYPE* type, const void* data, unsigned int data_len, void* buf)
{
	return ERR_NOERROR;
}

int protocol_binary_write(PROTOCOL_TYPE* type, const void* buf, void* data, unsigned int* data_len)
{
	return ERR_NOERROR;
}

int protocol_text_read(PROTOCOL_TYPE* type, const char* data, void* buf)
{
	return ERR_NOERROR;
}

int protocol_text_write(PROTOCOL_TYPE* type, const void* buf, char* data, unsigned int data_len)
{
	return ERR_NOERROR;
}

int protocol_file_read(PROTOCOL_TYPE* type, const char* filename, void* buf)
{
	int ret;
	char data[50*1024];

	ret = load_textfile(filename, data, sizeof(data));
	if(ret!=ERR_NOERROR)
		return ret;

	return protocol_text_read(type, data, buf);
}

int protocol_file_write(PROTOCOL_TYPE* type, const void* buf, const char* filename)
{
	int ret;
	char data[50*1024];

	ret = protocol_text_write(type, buf, data, sizeof(data));
	if(ret!=ERR_NOERROR)
		return ret;

	return save_textfile(filename, data, sizeof(data));
}
