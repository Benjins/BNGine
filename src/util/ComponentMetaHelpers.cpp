#include "../metagen/ComponentMeta.h"
#include "../metagen/MetaStruct.h"

#include "../../ext/CppUtils/strings.h"

int FindStructByName(const char* name) {
	for (int i = 0; i < CCT_Count; i++) {
		if (StrEqual(componentMetaData[i]->name, name)) {
			return i;
		}
	}

	return -1;
}

int FindEnumByName(const char* name) {
	for (int i = 0; i < CDE_Count; i++) {
		if (StrEqual(enumMetaData[i]->name, name)) {
			return i;
		}
	}

	return -1;
}

const char* EncodeEnum(int type, int val){
	ASSERT(type >= 0 && type < CDE_Count);
	for (int i = 0; i < enumMetaData[type]->entryCount; i++) {
		if (enumMetaData[type]->entries[i].value == val) {
			return enumMetaData[type]->entries[i].serial;
		}
	}

	return "<NO_ENUM_VALUE>";
}

int ParseEnum(int type, const char* str) {
	ASSERT(type >= 0 && type < CDE_Count);
	for (int i = 0; i < enumMetaData[type]->entryCount; i++) {
		if (StrEqual(str, enumMetaData[type]->entries[i].serial)) {
			return enumMetaData[type]->entries[i].value;
		}
	}

	return -1;
}
