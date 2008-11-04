
create table account(
	uuid integer primary key autoincrement,
	username text unique,
	password text,
	nick text unique default '',
	ri text default '',
	wh text default '',
	equ text default ''
);

create table account(uuid integer primary key autoincrement, username text unique, password text, nick text unique default NULL, ri text default '', wh text default '', equ text default '');

