#ifndef METASTRUCT_H
#define METASTRUCT_H

#pragma once

#include "MetaType.h"

enum FieldSerializeFlags {
	FSF_None = 0,
	FSF_SerializeFromId = (1 << 0),
	FSF_DoNotSerialize = (1 << 1),
	FSF_SerializeAsEnum = (1 << 2)
};

struct MetaAttribute {
	const char* name;
	const char** args;
	int argCount;
};

struct MetaField {
	const char* name;
	int offset;
	int type;

	const char* serializeExt;
	FieldSerializeFlags flags;
};

struct MetaEnumEntry {
	const char* name;
	const char* serial;
	int value;
};

enum MetaEnumFlags {
	MEF_None = 0,
	MEF_EnumIsFlag = (1 << 0)
};

struct MetaEnum {
	const char* name;
	MetaEnumEntry* entries;
	int entryCount;

	int allIdx;
	int noneIdex;
	MetaEnumFlags flags;
};

struct MetaStruct {
	const char* name;
	MetaStruct* parent;
	const MetaField* fields;
	int fieldCount;
	int size;
};

#endif
