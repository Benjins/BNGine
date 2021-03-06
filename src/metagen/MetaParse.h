#ifndef METAPARSER_H
#define METAPARSER_H

#pragma once

#include "MetaStruct.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/lexer.h"

#define NOT_AN_ARRAY -1

struct ParseMetaAttribute {
	SubString name;
	Vector<SubString> args;
};

struct ParseMetaField {
	SubString name;
	SubString type;
	SubString typeParam;
	int indirectionLevel;
	int arrayCount;
	Vector<ParseMetaAttribute> attrs;

	SubString serializeFromId;
	SubString serializeExt;
	FieldSerializeFlags flags;

	int enumType;

	ParseMetaField() {
		indirectionLevel = 0;
		arrayCount = NOT_AN_ARRAY;
		flags = FSF_None;
	}
};

struct ParseMetaEnumEntry {
	SubString name;
	Vector<SubString> value;
	Vector<ParseMetaAttribute> attrs;
};

struct ParseMetaEnum {
	SubString name;
	Vector<ParseMetaEnumEntry> entries;
	Vector<ParseMetaAttribute> attrs;

	int allIdx;
	int noneIdex;
	MetaEnumFlags flags;

	ParseMetaEnum() {
		allIdx = -1;
		noneIdex = -1;
		flags = MEF_None;
	}
};

struct ParseMetaFunction {
	SubString name;
};

struct ParseMetaStruct {
	SubString name;
	SubString parentName;
	Vector<ParseMetaField> fields;
	Vector<ParseMetaAttribute> attrs;
	Vector<ParseMetaFunction> methods;
};

Vector<ParseMetaStruct> ParseStructDefsFromFile(const char* fileName);

Vector<ParseMetaEnum> ParseEnumDefsFromFile(const char* fileName);

ParseMetaAttribute* FindMetaAttribByName(const Vector<ParseMetaAttribute>& attribs, const char* attribName);

Vector<ParseMetaAttribute> ParseMetaAttribsBackward(Vector<SubString>& tokens, int index, const char* fileName);

#endif
