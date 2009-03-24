#pragma once

typedef struct PDL_ARG {
	std::string name;
	std::string type;
	std::string size;
	std::string count;
	std::string desc;
} PDL_ARG;

typedef struct PDL_CMD {
	std::string name;
	std::string desc;
	std::vector<PDL_ARG> args;
} PDL_CMD;

extern std::vector<PDL_CMD> cmds;

extern bool pdl_parser(const char* text);
