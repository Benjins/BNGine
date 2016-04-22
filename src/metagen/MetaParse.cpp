#include "MetaParse.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/strings.h"


Vector<ParseMetaStruct> ParseStructDefsFromFile(const char* fileName) {
	int fileLength = 0;
	char* fileContents = ReadTextFile(fileName, &fileLength);

	Vector<Token> tokens = LexString(fileContents);

	Vector<ParseMetaStruct> parsedStructs;

	int braceCount = 0;
	bool inStruct = false;
	for (int i = 0; i < tokens.count; i++) {
		
		if (inStruct) {
			if (TOKEN_IS(tokens.data[i], "{")) {
				braceCount++;
			}
			else if (TOKEN_IS(tokens.data[i], "}")) {
				braceCount--;

				if (braceCount == 0) {
					inStruct = false;
				}
			}
			else if (TOKEN_IS(tokens.data[i], ";") && braceCount == 1) {
				Token prevTok = tokens.data[i-1];

				//If it's not a method declaration, we're assuming its a field declaration.
				//TODO: Could be a typedef, or an internal struct/enum
				if (!TOKEN_IS(prevTok, ")") && !TOKEN_IS(prevTok, "const")
					&& !TOKEN_IS(prevTok, "override") && !TOKEN_IS(prevTok, "final")) {
					ParseMetaField structField = { 0 };
					structField.arrayCount = NOT_AN_ARRAY;

					int fieldNameIndex = i - 1;

					if (TOKEN_IS(prevTok, "]")) {
						fieldNameIndex -= 3;
						structField.arrayCount = Atoi(tokens.data[i - 2].start);
					}

					structField.name = tokens.data[fieldNameIndex];

					int typeNameIndex = fieldNameIndex - 1;
					while (TOKEN_IS(tokens.data[typeNameIndex], "*")) {
						structField.indirectionLevel++;
						typeNameIndex--;
					}

					if (TOKEN_IS(tokens.data[typeNameIndex], ">")) {
						structField.typeParam = tokens.data[typeNameIndex - 1];
						typeNameIndex -= 3;
					}

					structField.type = tokens.data[typeNameIndex];

					parsedStructs.Get(parsedStructs.count - 1).fields.PushBack(structField);
				}
			}
		}
		else {
			if (TOKEN_IS(tokens.data[i], "struct") && !TOKEN_IS(tokens.data[i + 2], ";")) {
				inStruct = true;
				Token structName = tokens.data[i+1];
				ParseMetaStruct structDef = { 0 };
				structDef.name = structName;
				if (TOKEN_IS(tokens.data[i + 2], ":")) {
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

