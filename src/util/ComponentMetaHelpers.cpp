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