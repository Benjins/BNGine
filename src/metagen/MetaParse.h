#ifndef METAPARSER_H
#define METAPARSER_H

#pragma once

#include "MetaStruct.h"
#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/lexer.h"

struct ParseMetaAttribute {
	Token name;
	Vector<Token> args;
};

struct ParseMetaField {
	Token name;
	Token type;
	Token typeParam;
	int indirectionLevel;
	int arrayCount;
	Vector<ParseMetaAttribute> attrs;
};

struct ParseMetaFieldSub : ParseMetaField {
	int testyy[200];

	Vector<float> lols;

	Vector<Token> nott;

	int compOd;
};

#define NOT_AN_ARRAY -1

struct ParseMetaEnumEntry {
	Token value;
	Vector<ParseMetaAttribute> attrs;
};

struct ParseMetaEnum {
	Token name;
	Vector<ParseMetaEnumEntry> entries;
	Vector<ParseMetaAttribute> attrs;
};

struct ParseMetaStruct {
	Token name;
	Token parentName;
	Vector<ParseMetaField> fields;
	Vector<ParseMetaAttribute> attrs;
};

Vector<ParseMetaStruct> ParseStructDefsFromFile(const char* fileName);

Vector<ParseMetaEnum> ParseEnumDefsFromFile(const char* fileName);

#endif
