#include "MetaParse.h"

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/strings.h"

Vector<ParseMetaAttribute> ParseMetaAttribsBackward(Vector<SubString> tokens, int index, const char* fileName) {
	Vector<ParseMetaAttribute> attrs;
	for (int i = index; i >= 0; i--) {
		if (tokens.Get(i) == "]*/") {
			while (i >= 0 && tokens.Get(i) != "/*[") {
				i--;
			}

			if (i < 0) {
				printf("\nError: ']*/' without a corresponding '/*[' in '%s'\n", fileName);
			}
			else if (tokens.Get(i + 1) == "]*/") {
				printf("\nError: ']*/' immediately follwing '/*[' in '%s'\n", fileName);
			}
			else {
				ParseMetaAttribute attr;
				attr.name = tokens.Get(i + 1);
				if (tokens.Get(i + 2) == "(") {
					for (int j = i + 3; tokens.Get(j) != ")"; j++) {
						if (tokens.Get(j) == ",") {
							continue;
						}
						else if (tokens.Get(j) == "*]/") {
							printf("\nError: Attribute '%.*s' in file '%s' has no closing paren.\n", attr.name.length, attr.name.start, fileName);
							break;
						}
						else {
							attr.args.PushBack(tokens.Get(j));
						}
					}
				}

				attrs.PushBack(attr);
			}
		}
		else {
			break;
		}
	}

	return attrs;
}

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

					structField.attrs = ParseMetaAttribsBackward(tokens, typeNameIndex - 1, fileName);

					parsedStructs.Get(parsedStructs.count - 1).fields.PushBack(structField);
				}
				else if (prevTok == ")") {
					int j = i - 1;
					while (tokens.data[j] != "(") {
						j--;
					}

					SubString funcName = tokens.data[j - 1];
					ParseMetaFunction func;
					func.name = funcName;
					parsedStructs.Get(parsedStructs.count - 1).methods.PushBack(func);
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

				structDef.attrs = ParseMetaAttribsBackward(tokens, i - 1, fileName);

				parsedStructs.PushBack(structDef);
			}
		}
	}

	return parsedStructs;
}

ParseMetaAttribute* FindMetaAttribByName(const Vector<ParseMetaAttribute>& attribs, const char* attribName) {
	for (int i = 0; i < attribs.count; i++) {
		if (attribs.Get(i).name == attribName) {
			return &attribs.data[i];
		}
	}

	return nullptr;
}

Vector<ParseMetaEnum> ParseEnumDefsFromFile(const char* fileName) {
	return Vector<ParseMetaEnum>();
}

