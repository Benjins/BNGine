#ifndef METASTRUCT_H
#define METASTRUCT_H

#pragma once

#include "MetaType.h"

enum FieldSerializeFlags {
	FSF_None = 0,
	FSF_SerializeFromId = (1 << 0),
	FSF_DoNotSerialize = (1 << 1)
};

struct MetaAttribute {
	const char* name;
	const char** args;
	int argCount;
};

struct MetaField {
	const char* name;
	int offset;
	MetaType type;

	char* serializeExt;
	FieldSerializeFlags flags;
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
	int size;
};

#endif
