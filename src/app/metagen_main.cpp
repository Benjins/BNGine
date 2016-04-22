#include <stdio.h>

#include "../../ext/CppUtils/filesys.h"
#include "../metagen/MetaParse.h"


int main(int arc, char** argv) {
	File srcFiles;
	srcFiles.Load("src");

	Vector<File*> headerFiles;
	srcFiles.FindFilesWithExt("h", &headerFiles);

	Vector<ParseMetaStruct> allParseMetaStructs;

	for (int i = 0; i < headerFiles.count; i++) {
		Vector<ParseMetaStruct> fileParseMetaStructs = ParseStructDefsFromFile(headerFiles.data[i]->fullName);

		allParseMetaStructs.EnsureCapacity(allParseMetaStructs.count + fileParseMetaStructs.count);

		for (int j = 0; j < fileParseMetaStructs.count; j++) {
			allParseMetaStructs.PushBack(fileParseMetaStructs.data[j]);
		}
	}

	for (int i = 0; i < allParseMetaStructs.count; i++) {
		ParseMetaStruct parseMeta = allParseMetaStructs.data[i];
		printf("struct %.*s", parseMeta.name.length, parseMeta.name.start);
		if (parseMeta.parentName.start != nullptr) {
			printf(" : %.*s", parseMeta.parentName.length, parseMeta.parentName.start);
		}

		printf("{\n");

		for (int j = 0; j < parseMeta.fields.count; j++) {
			ParseMetaField mf = parseMeta.fields.data[j];
			printf("\t%.*s", mf.type.length, mf.type.start);

			if (mf.typeParam.start != nullptr) {
				printf("<%.*s>", mf.typeParam.length, mf.typeParam.start);
			}

			for (int k = 0; k < mf.indirectionLevel; k++) {
				printf("*");
			}

			printf(" %.*s", mf.name.length, mf.name.start);

			if (mf.arrayCount != NOT_AN_ARRAY) {
				printf("[%d]", mf.arrayCount);
			}

			printf(";\n");
		}

		printf("};\n\n");
	}

	return 0;
}



