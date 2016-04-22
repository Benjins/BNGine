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

	ParseMetaField() {
		indirectionLevel = 0;
		arrayCount = NOT_AN_ARRAY;
	}
};

struct ParseMetaFieldSub : ParseMetaField {
	int testyy[200];

	Vector<float> lols;

	Vector<SubString> nott;

	int compOd;
};

struct ParseMetaEnumEntry {
	SubString value;
	Vector<ParseMetaAttribute> attrs;
};

struct ParseMetaEnum {
	SubString name;
	Vector<ParseMetaEnumEntry> entries;
	Vector<ParseMetaAttribute> attrs;
};

struct ParseMetaStruct {
	SubString name;
	SubString parentName;
	Vector<ParseMetaField> fields;
	Vector<ParseMetaAttribute> attrs;
};

Vector<ParseMetaStruct> ParseStructDefsFromFile(const char* fileName);

Vector<ParseMetaEnum> ParseEnumDefsFromFile(const char* fileName);

#endif
