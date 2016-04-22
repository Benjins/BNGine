#ifndef METASTRUCT_H
#define METASTRUCT_H

#pragma once

#include "MetaType.h"

struct MetaAttribute {
	const char* name;
	const char** args;
	int argCount;
};

struct MetaField {
	const char* name;
	int offset;
	MetaType type;
};

struct MetaEnumEntry {
	const char* value;
	const char* serial;
};

struct MetaEnum {
	const char* name;
	MetaEnumEntry* entries;
	int entryCount;
};

struct MetaStruct {
	const char* name;
	MetaStruct* parent;
	const MetaField* fields;
	int fieldCount;
};

#endif
