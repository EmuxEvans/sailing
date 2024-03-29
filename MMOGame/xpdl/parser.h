#pragma once

typedef struct PDL_ARG {
	std::string name;
	bool isarray;
	std::string type;
	std::string range;
} PDL_ARG;

typedef struct PDL_CMD {
	std::string mode;
	std::string code;
	std::string name;
	std::vector<PDL_ARG> args;
} PDL_CMD;

extern std::vector<PDL_CMD> cmds;

extern bool pdl_parser(const char* text);
