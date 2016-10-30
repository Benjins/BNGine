#include <stdio.h>

#include "../../ext/CppUtils/filesys.h"
#include "../../ext/CppUtils/stringmap.h"
#include "../../ext/CppUtils/Coroutines/coroutines.h"
#include "../../ext/CppUtils/xml.h"
#include "../metagen/MetaParse.h"

// Bleh, this should be unified at some point anyway
struct ParseMetaFuncDef : MetaFuncDef {
	Vector<ParseMetaAttribute> attribs;
};


void ParseFunctionHeadersFromFile(const char* fileName, Vector<ParseMetaFuncDef>* outFuncDefs) {
	String fileContents = ReadStringFromFile(fileName);
	Vector<SubString> lexedFile = LexString(fileContents);

	for (int i = 0; i < lexedFile.count; i++) {
		ParseMetaFuncDef def;
		int end = ParseFuncHeader(lexedFile, i, &def);

		if (end > i && end < lexedFile.count - 1 && lexedFile.data[end + 1] == ";") {
			def.attribs = ParseMetaAttribsBackward(lexedFile, i - 1, fileName);
			outFuncDefs->PushBack(def);
			i = end + 1;
		}
	}
}

MetaType ParseType(const SubString& name, int indirectionLevel, int arrayCount) {
	if (arrayCount == NOT_AN_ARRAY) {
		if (indirectionLevel == 0) {
			if (name == "int") {
				return MT_Int;
			}
			else if (name == "float") {
				return MT_Float;
			}
			else if (name == "bool") {
				return MT_Bool;
			}
			else if (name == "String") {
				return MT_StringRef;
			}
			else if (name == "SubString") {
				return MT_SubStringRef;
			}
			else if (name == "Vector2") {
				return MT_Vector2;
			}
			else if (name == "Vector3") {
				return MT_Vector3;
			}
			else if (name == "Vector4") {
				return MT_Vector4;
			}
			else if (name == "Quaternion") {
				return MT_Quaternion;
			}
			else {
				return MT_Unknown;
			}
		}
		else if (indirectionLevel == 1) {
			if (name == "char") {
				return MT_String;
			}
		}
		else {
			return MT_Unknown;
		}
	}

	return MT_Unknown;
}

void DefineCustomComponentFunction(
	FILE* componentMetaFile, 
	const Vector<int>* componentIndices, 
	const Vector<ParseMetaStruct>* allParseMetaStructs,
	const Vector<String>* getComponentPathList,
	const char* functionName,
	const char* methodName,
	bool singleEntity = false)
{
	fprintf(componentMetaFile, "\nvoid %s(%s){\n", functionName, (singleEntity ? "uint32 entId" : ""));
	for (int i = 0; i < componentIndices->count; i++) {
		int compIdx = componentIndices->data[i];
		ParseMetaStruct* ms = &allParseMetaStructs->data[compIdx];

		bool hasUpdate = false;
		for (int j = 0; j < ms->methods.count; j++) {
			if (ms->methods.Get(j).name == methodName) {
				hasUpdate = true;
				break;
			}
		}

		if (hasUpdate) {
			fprintf(componentMetaFile, "\tfor (int i = 0; i < %s.currentCount; i++) {\n", 
				getComponentPathList->Get(i).string);
			if (singleEntity) {
				fprintf(componentMetaFile, "\t\tif (%s.vals[i].entity == entId){\n",
					getComponentPathList->Get(i).string);
				fprintf(componentMetaFile, "\t\t\t%s.vals[i].%s();\n",
					getComponentPathList->Get(i).string, methodName);
				fprintf(componentMetaFile, "\t\t}\n");
			}
			else {
				fprintf(componentMetaFile, "\t\t%s.vals[i].%s();\n", 
					getComponentPathList->Get(i).string, methodName);
			}
			fprintf(componentMetaFile, "\t}\n");
		}
	}
	fprintf(componentMetaFile, "\n}\n");
}

void DumpTypeInfo(MetaTypeInfo typeInfo, FILE* fp) {
	if (typeInfo.isConst) {
		fprintf(fp, "const ");
	}

	fprintf(fp, "%.*s", typeInfo.typeName.length, typeInfo.typeName.start);
	if (typeInfo.genericParam.length > 0) {
		fprintf(fp, "<%.*s>", typeInfo.genericParam.length, typeInfo.genericParam.start);
	}

	for (int i = 0; i < typeInfo.pointerLevel; i++) {
		fprintf(fp, "*");
	}

	if (typeInfo.isReference) {
		fprintf(fp, "&");
	}
}

void DumpFunctionHeader(MetaFuncDef* def, FILE* fp) {
	DumpTypeInfo(def->retType, fp);
	fprintf(fp, " %.*s(", def->name.length, def->name.start);

	for (int i = 0; i < def->params.count; i++) {
		if (i > 0) {
			fprintf(fp, ", ");
		}

		MetaVarDecl param = def->params.data[i];
		char declBuffer[256];
		SerializeVarDecl(&param.type, param.name.start, param.name.length, declBuffer, sizeof(declBuffer));
		fprintf(fp, "%s", declBuffer);
	}

	fprintf(fp, ");");
}

bool FuncDefHasAttrib(ParseMetaFuncDef* def, const char* attr, ParseMetaAttribute* outAttrib = nullptr) {
	for (int i = 0; i < def->attribs.count; i++) {
		if (def->attribs.data[i].name == attr) {
			if (outAttrib != nullptr) {
				*outAttrib = def->attribs.data[i];
			}
			return true;
		}
	}

	return false;
}

int main(int arc, char** argv) {
	File srcFiles;
	srcFiles.Load("src");

	Vector<File*> headerFiles;
	srcFiles.FindFilesWithExt("h", &headerFiles);

	Vector<ParseMetaStruct> allParseMetaStructs;
	Vector<ParseMetaFuncDef> allParseMetaFuncs;

	StringMap<String> structToFileMap;

	for (int i = 0; i < headerFiles.count; i++) {
		Vector<ParseMetaStruct> fileParseMetaStructs = ParseStructDefsFromFile(headerFiles.data[i]->fullName);

		allParseMetaStructs.EnsureCapacity(allParseMetaStructs.count + fileParseMetaStructs.count);

		for (int j = 0; j < fileParseMetaStructs.count; j++) {
			structToFileMap.Insert(fileParseMetaStructs.data[j].name, headerFiles.data[i]->fullName);
			allParseMetaStructs.PushBack(fileParseMetaStructs.data[j]);
		}

		Vector<ParseMetaFuncDef> fileParseFuncDefs;
		ParseFunctionHeadersFromFile(headerFiles.data[i]->fullName, &fileParseFuncDefs);
		allParseMetaFuncs.InsertVector(allParseMetaFuncs.count, fileParseFuncDefs);
	}

	StringMap<int> typeIndices;

	Vector<int> componentIndices;

	for (int i = 0; i < allParseMetaStructs.count; i++) {
		typeIndices.Insert(allParseMetaStructs.data[i].name, i);

		// HACK: Transform probably is a special case we'll just do manually.
		if (allParseMetaStructs.data[i].name != "Transform" && allParseMetaStructs.data[i].parentName == "Component") {
			componentIndices.PushBack(i);
		}
	}

	static const char* parseFuncs[MT_FundamentalCount] =
	{
		"Atoi",
		"atof",
		"ParseBool",
		"",
		"",
		"",
		"ParseVector2",
		"ParseVector3",
		"ParseVector4",
		"ParseQuaternion"
	};

	static const char* encodeFuncs[MT_FundamentalCount] =
	{
		"Itoa",
		"Ftoa",
		"EncodeBool",
		"",
		"",
		"",
		"EncodeVector2",
		"EncodeVector3",
		"EncodeVector4",
		"EncodeQuaternion"
	};

	//ComponentType.h

	FILE* componentTypeFile = fopen("gen/ComponentType.h", "wb");

	fprintf(componentTypeFile, "#ifndef COMPONENTTYPE_H\n");
	fprintf(componentTypeFile, "#define COMPONENTTYPE_H\n");
	fprintf(componentTypeFile, "\n#pragma once\n\n");

	fprintf(componentTypeFile, "enum CustomComponentType{\n");
	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];
		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];

		fprintf(componentTypeFile, "\tCCT_%.*s,\n", ms->name.length, ms->name.start);
	}
	fprintf(componentTypeFile, "\tCCT_Count\n");
	fprintf(componentTypeFile, "};\n\n");

	fprintf(componentTypeFile, "#endif");

	fclose(componentTypeFile);

	ParseMetaStruct* sceneStruct = nullptr;
	ParseMetaStruct* levelStruct = nullptr;

	{
		int sceneIndex = -1;
		if (typeIndices.LookUp("Scene", &sceneIndex)) {
			sceneStruct = &allParseMetaStructs.data[sceneIndex];
		}
	}

	{
		int levelIndex = -1;
		if (typeIndices.LookUp("Level", &levelIndex)) {
			levelStruct = &allParseMetaStructs.data[levelIndex];
		}
	}

	ASSERT(sceneStruct != nullptr);
	ASSERT(levelStruct != nullptr);

	Vector<String> getComponentPathList;
	Vector<String> getComponentLevelPathList;
	
	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];
		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];
		
		for (int j = 0; j < sceneStruct->fields.count; j++) {
			int sceneFieldIdx = -1;
			if (typeIndices.LookUp(sceneStruct->fields.data[j].type, &sceneFieldIdx)) {
				ParseMetaStruct* fms = &allParseMetaStructs.data[sceneFieldIdx];
				for (int k = 0; k < fms->fields.count; k++) {
					ParseMetaField mf = fms->fields.data[k];
					if (mf.typeParam == ms->name) {
						StringStackBuffer<256> getCompPath("GlobalScene->%.*s.%.*s", 
							sceneStruct->fields.data[j].name.length, 
							sceneStruct->fields.data[j].name.start,
							mf.name.length, 
							mf.name.start);
						getComponentPathList.PushBack(getCompPath.buffer);
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];
		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];

		for (int j = 0; j < levelStruct->fields.count; j++) {
			int levelFieldIdx = -1;
			ParseMetaField levelField = levelStruct->fields.data[j];
			if (levelField.type == "Vector"
				&& typeIndices.LookUp(levelField.typeParam, &levelFieldIdx)) {
				ParseMetaStruct* fms = &allParseMetaStructs.data[levelFieldIdx];
				if (fms == ms) {
					StringStackBuffer<256> getCompPath("%.*s", levelField.name.length, levelField.name.start);
					getComponentLevelPathList.PushBack(getCompPath.buffer);
					break;
				}
			}
		}
	}

	ASSERT(getComponentPathList.count == componentIndices.count);
	ASSERT(getComponentLevelPathList.count == componentIndices.count);

	//ComponentReset.cpp

	FILE* componentResetFile = fopen("gen/ComponentReset.cpp", "wb");

	fprintf(componentResetFile, "#include \"../src/core/Scene.h\"\n");
	fprintf(componentResetFile, "#include \"../src/util/LevelLoading.h\"\n\n");

	fprintf(componentResetFile, "void Scene::ResetComponents(){\n");
	for (int i = 0; i < getComponentPathList.count; i++) {
		fprintf(componentResetFile, "\t%s.Reset();\n", getComponentPathList.data[i].string);
	}
	fprintf(componentResetFile, "}\n");

	fprintf(componentResetFile, "void Scene::LoadCustomComponentsFromLevel(const Level* level){\n");
	for (int i = 0; i < getComponentPathList.count; i++) {
		SubString compTypeName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentResetFile, "\tLoadVectorToIDTracker<%.*s>(level->%s, %s);\n", compTypeName.length, compTypeName.start,
			getComponentLevelPathList.data[i].string, getComponentPathList.data[i].string);
	}
	fprintf(componentResetFile, "}\n");

	fprintf(componentResetFile, "void Scene::SaveCustomComponentsToLevel(Level* level){\n");
	for (int i = 0; i < getComponentPathList.count; i++) {
		SubString compTypeName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentResetFile, "\tLoadIDTrackerToVector<%.*s>(%s, level->%s);\n", compTypeName.length, compTypeName.start,
			getComponentPathList.data[i].string, getComponentLevelPathList.data[i].string);
	}
	fprintf(componentResetFile, "}\n");

	fprintf(componentResetFile, "void Scene::DestroyCustomComponentsByEntity(uint32 entId){\n");
	for (int i = 0; i < getComponentPathList.count; i++) {
		SubString compTypeName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentResetFile, "\tfor (int i = 0; i < %s.currentCount; i++){\n", getComponentPathList.data[i].string);
		fprintf(componentResetFile, "\t\tComponent* comp = &%s.vals[i];\n", getComponentPathList.data[i].string);
		fprintf(componentResetFile, "\t\tif (comp->entity == entId){\n");
		fprintf(componentResetFile, "\t\t\t%s.RemoveById(comp->id);\n", getComponentPathList.data[i].string);
		fprintf(componentResetFile, "\t\t\tbreak;\n");
		fprintf(componentResetFile, "\t\t}\n");
		fprintf(componentResetFile, "\t}\n");
	}
	fprintf(componentResetFile, "}\n");

	//void SendCollisionToCustomComponents(uint32 entity, Collision col);

	fprintf(componentResetFile, "\nvoid Scene::SendCollisionToCustomComponents(uint32 entityId, Collision col){\n");
	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];
		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];

		bool hasCollision = false;
		for (int j = 0; j < ms->methods.count; j++) {
			if (ms->methods.Get(j).name == "OnCollision") {
				hasCollision = true;
				break;
			}
		}

		if (hasCollision) {
			fprintf(componentResetFile, "\t{\n");
			fprintf(componentResetFile, "\t\t%.*s* %.*s_comp = FIND_COMPONENT_BY_ENTITY(%.*s, entityId);\n",
				ms->name.length, ms->name.start, ms->name.length, ms->name.start, ms->name.length, ms->name.start);
			fprintf(componentResetFile, "\t\tif (%.*s_comp != nullptr){\n", ms->name.length, ms->name.start);
			fprintf(componentResetFile, "\t\t\t%.*s_comp->OnCollision(col);\n", ms->name.length, ms->name.start);
			fprintf(componentResetFile, "\t\t}\n");
			fprintf(componentResetFile, "\t}\n");
		}
	}
	fprintf(componentResetFile, "\n}\n");

	fclose(componentResetFile);

	//ComponentMeta.cpp

	FILE* componentMetaFile = fopen("gen/ComponentMeta.cpp", "wb");

	fprintf(componentMetaFile, "#include \"../src/core/Scene.h\"\n");
	fprintf(componentMetaFile, "#include \"../src/core/Component.h\"\n");
	fprintf(componentMetaFile, "#include \"../src/metagen/MetaStruct.h\"\n");
	fprintf(componentMetaFile, "#include \"../src/metagen/ComponentMeta.h\"\n");
	fprintf(componentMetaFile, "#include \"../src/util/Serialization.h\"\n\n");

	fprintf(componentMetaFile, "#include \"../ext/CppUtils/xml.h\"\n");
	fprintf(componentMetaFile, "#include \"../ext/CppUtils/memstream.h\"\n");

	fprintf(componentMetaFile, "#include \"ComponentType.h\"\n\n");

	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];
		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];

		String includefile;
		structToFileMap.LookUp(ms->name, &includefile);
		
		fprintf(componentMetaFile, "#include \"../%s\"//%.*s\n", includefile.string, ms->name.length, ms->name.start);
	}

	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];
		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];

		fprintf(componentMetaFile, "Component* %.*s_getComponentArray(){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\treturn %s.vals;\n", getComponentPathList.data[i].string);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "int %.*s_getComponentCount(){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\treturn %s.currentCount;\n", getComponentPathList.data[i].string);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "Component* %.*s_getComponentById(uint32 id){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\treturn %s.GetById(id);\n", getComponentPathList.data[i].string);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "Component* %.*s_createAndAdd(){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tComponent* comp = %s.CreateAndAdd();\n", getComponentPathList.data[i].string);
		fprintf(componentMetaFile, "\tcomp->type = CCT_%.*s;\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\treturn comp;\n");
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "void %.*s_Remove(uint32 id){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t%s.RemoveById(id);\n", getComponentPathList.data[i].string);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "Component* %.*s_getLevelArray(const Level* lvl){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tComponent* comps = lvl->%s.data;\n", getComponentLevelPathList.data[i].string);
		fprintf(componentMetaFile, "\treturn comps;\n");
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "int %.*s_getLevelCount(const Level* lvl){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tint count = lvl->%s.count;\n", getComponentLevelPathList.data[i].string);
		fprintf(componentMetaFile, "\treturn count;\n");
		fprintf(componentMetaFile, "}\n\n");
	}

	DefineCustomComponentFunction(componentMetaFile, &componentIndices, &allParseMetaStructs,
		&getComponentPathList, "Scene::UpdateCustomComponents", "Update");
	DefineCustomComponentFunction(componentMetaFile, &componentIndices, &allParseMetaStructs,
		&getComponentPathList, "Scene::CustomComponentEditorGui", "EditorGui");
	DefineCustomComponentFunction(componentMetaFile, &componentIndices, &allParseMetaStructs,
		&getComponentPathList, "Scene::CustomComponentEditorGuiForEntity", "EditorGui", true);

	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];
		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];

		int usedFieldCount = 0;

		fprintf(componentMetaFile, "MetaField %.*s_metaFields[] = {\n", ms->name.length, ms->name.start);
		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount);
			if (fieldType != MT_Unknown) {
				usedFieldCount++;
				fprintf(componentMetaFile, "\t{\"%.*s\", (int)(size_t)(&((%.*s*)0)->%.*s), (MetaType)%d},\n",
					mf.name.length, mf.name.start,
					ms->name.length, ms->name.start,
					mf.name.length, mf.name.start, 
					(int)fieldType);
			}
		}
		fprintf(componentMetaFile, "};\n\n");

		fprintf(componentMetaFile, "MetaStruct %.*s_meta = {\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t\"%.*s\",\n", ms->name.length, ms->name.start);

		if (ms->parentName.start != nullptr && ms->parentName != "Component") {
			int parentIdx = -1;
			typeIndices.LookUp(ms->parentName, &parentIdx);
			SubString parentName = allParseMetaStructs.data[parentIdx].name;
			fprintf(componentMetaFile, "\t%.*s_meta,\n", parentName.length, parentName.start);
		}
		else {
			fprintf(componentMetaFile, "\tnullptr,\n");
		}

		fprintf(componentMetaFile, "\t%.*s_metaFields,\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t%d,\n", usedFieldCount);
		fprintf(componentMetaFile, "\tsizeof(%.*s)\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "};\n\n");
	}

	for (int i = 0; i < componentIndices.count; i++) {
		int compIdx = componentIndices.data[i];

		ParseMetaStruct* ms = &allParseMetaStructs.data[compIdx];
		//printf("Comp child: %.*s\n", ms->name.length, ms->name.start);

		//Component*, const XMLElement*
		fprintf(componentMetaFile, "void %.*s_XMLDeserialize(Component* comp, const XMLElement* elem){\n", ms->name.length, ms->name.start);

		fprintf(componentMetaFile, "\t%.*s* compCast = static_cast<%.*s*>(comp);\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);

		fprintf(componentMetaFile, "\tString temp;\n");

		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount);

			if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				fprintf(componentMetaFile, "\tif(elem->attributes.LookUp(\"%.*s\", &temp)){\n", mf.name.length, mf.name.start);
				fprintf(componentMetaFile, "\t\tcompCast->%.*s = %s(temp.string);\n\t}\n", mf.name.length, mf.name.start, parseFuncs[(int)fieldType]);
			}
		}

		fprintf(componentMetaFile, "}\n\n");
		fprintf(componentMetaFile, "void %.*s_XMLSerialize(const Component* comp, XMLElement* elem){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tconst %.*s* compCast = static_cast<const %.*s*>(comp);\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\telem->name = STATIC_TO_SUBSTRING(\"%.*s\");\n", ms->name.length, ms->name.start);

		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount);

			if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				fprintf(componentMetaFile, "\telem->attributes.Insert(\"%.*s\", %s(compCast->%.*s));\n", mf.name.length, mf.name.start, encodeFuncs[(int)fieldType], mf.name.length, mf.name.start);
			}
		}

		fprintf(componentMetaFile, "}\n\n");
		fprintf(componentMetaFile, "void %.*s_MemSerialize(const Component* comp, MemStream* stream){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tconst %.*s* compCast = static_cast<const %.*s*>(comp);\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);
		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount);

			if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				fprintf(componentMetaFile, "\tstream->Write<%.*s>(compCast->%.*s);\n", mf.type.length, mf.type.start, mf.name.length, mf.name.start);
			}
		}
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "void %.*s_MemDeserialize(Component* comp, MemStream* stream){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t%.*s* compCast = static_cast<%.*s*>(comp);\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);
		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount);

			if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				fprintf(componentMetaFile, "\tcompCast->%.*s = stream->Read<%.*s>();\n", mf.name.length, mf.name.start, mf.type.length, mf.type.start);
			}
		}
		fprintf(componentMetaFile, "}\n\n");

		const char* lvlPath = getComponentLevelPathList.Get(i).string;
		fprintf(componentMetaFile, "Component* %.*s_addToLevel(Level* lvl){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tlvl->%s.PushBack(%.*s());\n", lvlPath, ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\treturn &lvl->%s.data[lvl->%s.count-1];\n", lvlPath, lvlPath);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "void %.*s_ResetComp(Component* comp){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tmemset(comp, 0, sizeof(%.*s));\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tnew (comp) %.*s();\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "%.*s %.*s_compBuffer;\n\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);

	}

	fprintf(componentMetaFile, "AddComponentToLevelFunc* addComponentToLevelFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_addToLevel,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "GetComponentArrayFunc* getComponentArrayFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_getComponentArray,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "ResetComponentFunc* componentResetFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_ResetComp,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "Component* componentSerializeBuffer[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t&%.*s_compBuffer,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "GetComponentCountFunc* getComponentCountFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_getComponentCount,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "AddComponentFunc* addComponentFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_createAndAdd,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "RemoveComponentFunc* removeComponentFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_Remove,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "ComponentXMLDeserializeFunc* componentXMLDeserializeFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_XMLDeserialize,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "ComponentXMLSerializeFunc* componentXMLSerializeFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_XMLSerialize,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "ComponentMemDeserializeFunc* componentMemDeserializeFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_MemDeserialize,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "ComponentMemSerializeFunc* componentMemSerializeFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_MemSerialize,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "GetComponentLevelArrayFunc* getComponentLevelArrayFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_getLevelArray,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "GetComponentLevelCountFunc* getComponentLevelCountFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_getLevelCount,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fprintf(componentMetaFile, "MetaStruct* componentMetaData[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t&%.*s_meta,\n", structName.length, structName.start);
	}
	fprintf(componentMetaFile, "};\n");

	fclose(componentMetaFile);

	FILE* actionHeaderFile = fopen("gen/Actions.h", "wb");

	Vector <ParseMetaFuncDef> funcsWithActionAttrib;

	BNS_VEC_FILTER(allParseMetaFuncs, funcsWithActionAttrib, FuncDefHasAttrib(&item, "Action"));

	fprintf(actionHeaderFile, "#ifndef ACTION_H\n");
	fprintf(actionHeaderFile, "#define ACTION_H\n\n");
	fprintf(actionHeaderFile, "#pragma once\n\n");

	fprintf(actionHeaderFile, "#include \"../ext/CppUtils/assert.h\"\n\n");

	for (int i = 0; i < funcsWithActionAttrib.count; i++) {
		DumpFunctionHeader(&funcsWithActionAttrib.data[i], actionHeaderFile);
		fprintf(actionHeaderFile, "\n");
	}

	fprintf(actionHeaderFile, "enum ActionType {\n");
	for (int i = 0; i < funcsWithActionAttrib.count; i++) {
		ParseMetaFuncDef* def = &funcsWithActionAttrib.data[i];
		fprintf(actionHeaderFile, "\tAT_%.*s,\n", def->name.length, def->name.start);
	}
	fprintf(actionHeaderFile, "\tAT_Count\n");
	fprintf(actionHeaderFile, "};\n\n");

	fprintf(actionHeaderFile, "struct Action {\n");
	fprintf(actionHeaderFile, "\tActionType type;\n");

	fprintf(actionHeaderFile, "\tunion {\n");

	for (int i = 0; i < funcsWithActionAttrib.count; i++) {
		ParseMetaFuncDef* def = &funcsWithActionAttrib.data[i];
		fprintf(actionHeaderFile, "\t\tstruct {\n");

		for (int j = 0; j < def->params.count; j++) {
			char buffer[256] = {};
			MetaVarDecl* param = &def->params.data[j];
			SerializeVarDecl(&param->type, param->name.start, param->name.length, buffer, sizeof(buffer));
			fprintf(actionHeaderFile, "\t\t\t%s;\n", buffer);
		}

		fprintf(actionHeaderFile, "\t\t} %.*s_data;\n", def->name.length, def->name.start);
	}

	fprintf(actionHeaderFile, "\t};\n");

	fprintf(actionHeaderFile, "};\n\n");

	fprintf(actionHeaderFile, "inline void ExecuteAction(const Action action){\n");
	fprintf(actionHeaderFile, "switch (action.type){\n");

	for (int i = 0; i < funcsWithActionAttrib.count; i++) {
		ParseMetaFuncDef* def = &funcsWithActionAttrib.data[i];
		fprintf(actionHeaderFile, "\tcase AT_%.*s: {\n", def->name.length, def->name.start);

		fprintf(actionHeaderFile, "\t\t%.*s(", def->name.length, def->name.start);
		for (int j = 0; j < def->params.count; j++) {
			if (j > 0) {
				fprintf(actionHeaderFile, ", ");
			}

			fprintf(actionHeaderFile, "action.%.*s_data.%.*s",
				def->name.length, def->name.start, 
				def->params.data[j].name.length, def->params.data[j].name.start);
		}
		fprintf(actionHeaderFile, ");\n");

		fprintf(actionHeaderFile, "\t} break;\n");
	}

	fprintf(actionHeaderFile, "\tdefault:{ASSERT_WARN(\"%%s\", \"Incorrect value for action!\");break;}\n");

	fprintf(actionHeaderFile, "}\n");
	fprintf(actionHeaderFile, "}\n\n");

	fprintf(actionHeaderFile, "#endif\n");
	fclose(actionHeaderFile);

	return 0;
}



