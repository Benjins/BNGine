#include "MetaParse.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/strings.h"


Vector<ParseMetaStruct> ParseStructDefsFromFile(const char* fileName) {
	String fileContents = ReadStringFromFile(fileName);

	Vector<SubString> tokens = LexString(fileContents);

	Vector<ParseMetaStruct> parsedStructs;

	int braceCount = 0;
	bool inStruct = false;
	for (int i = 0; i < tokens.count; i++) {
		
		if (inStruct) {
			if (tokens.data[i] == "{") {
				braceCount++;
			}
			else if (tokens.data[i] == "}") {
				braceCount--;

				if (braceCount == 0) {
					inStruct = false;
				}
			}
			else if (tokens.data[i] == ";" && braceCount == 1) {
				SubString prevTok = tokens.data[i-1];

				//If it's not a method declaration, we're assuming its a field declaration.
				//TODO: Could be a typedef
				if (prevTok != ")" && prevTok != "const" && prevTok != "override" && prevTok != "final" && prevTok != "}"){
					ParseMetaField structField;

					int fieldNameIndex = i - 1;

					if (prevTok == "]") {
						fieldNameIndex -= 3;
						structField.arrayCount = Atoi(tokens.data[i - 2].start);
					}

					structField.name = tokens.data[fieldNameIndex];

					int typeNameIndex = fieldNameIndex - 1;
					while (tokens.data[typeNameIndex] == "*") {
						structField.indirectionLevel++;
						typeNameIndex--;
					}

					if (tokens.data[typeNameIndex] == ">") {
						structField.typeParam = tokens.data[typeNameIndex - 1];
						typeNameIndex -= 3;
					}

					structField.type = tokens.data[typeNameIndex];

					parsedStructs.Get(parsedStructs.count - 1).fields.PushBack(structField);
				}
			}
		}
		else {
			if (tokens.data[i] == "struct" && tokens.data[i + 2] != ";") {
				inStruct = true;
				SubString structName = tokens.data[i+1];
				ParseMetaStruct structDef;
				structDef.name = structName;
				if (tokens.data[i + 2] == ":") {
					structDef.parentName = tokens.data[i + 3];
				}

				parsedStructs.PushBack(structDef);
			}
		}
	}

	return parsedStructs;
}

Vector<ParseMetaEnum> ParseEnumDefsFromFile(const char* fileName) {
	return Vector<ParseMetaEnum>();
}

