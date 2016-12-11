#include <stdio.h>

#include "app_funcs.h"

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

		if (end > i && end < lexedFile.count - 1 
			&& (lexedFile.data[end + 1] == "{" || lexedFile.data[end + 1] == ";")) {
			def.attribs = ParseMetaAttribsBackward(lexedFile, i - 1, fileName);
			outFuncDefs->PushBack(def);
			i = end + 1;
		}
	}
}

MetaType ParseType(const SubString& name, int indirectionLevel, int arrayCount, const Vector<ParseMetaEnum>& enumDefs, SubString* outEnumName) {
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
			else if (name == "IDHandle") {
				return MT_Handle;
			}
			else {
				for (int i = 0; i < enumDefs.count; i++) {
					if (enumDefs.data[i].name == name) {
						*outEnumName = enumDefs.data[i].name;
						return MT_CustomEnum;
					}
				}
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
	fprintf(componentMetaFile, "\nvoid %s(%s){\n", functionName, (singleEntity ? "IDHandle<Entity> entId" : ""));
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

bool MetaFieldHasAttrib(ParseMetaField* mf, const char* attr, ParseMetaAttribute* outAttrib = nullptr) {
	for (int i = 0; i < mf->attrs.count; i++) {
		if (mf->attrs.data[i].name == attr) {
			if (outAttrib != nullptr) {
				*outAttrib = mf->attrs.data[i];
			}
			return true;
		}
	}

	return false;
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

bool EnumHasAttrib(ParseMetaEnum* def, const char* attr, ParseMetaAttribute* outAttrib = nullptr) {
	for (int i = 0; i < def->attrs.count; i++) {
		if (def->attrs.data[i].name == attr) {
			if (outAttrib != nullptr) {
				*outAttrib = def->attrs.data[i];
			}
			return true;
		}
	}

	return false;
}

bool EnumEntryHasAttrib(ParseMetaEnumEntry* entry, const char* attr, ParseMetaAttribute* outAttrib = nullptr) {
	for (int i = 0; i < entry->attrs.count; i++) {
		if (entry->attrs.data[i].name == attr) {
			if (outAttrib != nullptr) {
				*outAttrib = entry->attrs.data[i];
			}
			return true;
		}
	}

	return false;
}

void AppPostInit(int argc, char** argv) {}
bool AppUpdate(int argc, char** argv) { return false; }
void AppShutdown(int argc, char** argv) {}
void AppMouseMove(int x, int y){}
void AppMouseUp(int button) {}
void AppMouseDown(int button) {}
void AppKeyUp(unsigned char key) {}
void AppKeyDown(unsigned char key) {}

void AppPreInit(int argc, char** argv){
	File srcFiles;
	srcFiles.Load("src");

	Vector<File*> headerFiles;
	srcFiles.FindFilesWithExt("h", &headerFiles);

	Vector<ParseMetaStruct> allParseMetaStructs;
	Vector<ParseMetaEnum> allParseMetaEnums;
	Vector<ParseMetaFuncDef> allParseMetaFuncs;

	StringMap<String> structToFileMap;

	Vector<String> additionalIncludes;

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

		Vector<ParseMetaEnum> fileParseEnumDefs = ParseEnumDefsFromFile(headerFiles.data[i]->fullName);
		allParseMetaEnums.InsertVector(allParseMetaEnums.count, fileParseEnumDefs);

		if (fileParseEnumDefs.count > 0) {
			additionalIncludes.PushBack(headerFiles.data[i]->fullName);
		}
	}

	StringMap<int> typeIndices;

	Vector<int> componentIndices;

	for (int i = 0; i < allParseMetaStructs.count; i++) {
		typeIndices.Insert(allParseMetaStructs.data[i].name, i);

		// HACK: Transform probably is a special case we'll just do manually.
		if (allParseMetaStructs.data[i].name != "Transform" && allParseMetaStructs.data[i].parentName == "Component") {
			componentIndices.PushBack(i);
			
			for (int j = 0; j < allParseMetaStructs.data[i].fields.count; j++) {
				ParseMetaField* mf = &allParseMetaStructs.data[i].fields.data[j];
				ParseMetaAttribute attrib;
				if (MetaFieldHasAttrib(mf, "DoNotSerialize")) {
					mf->flags = (FieldSerializeFlags)((int)mf->flags | FSF_DoNotSerialize);
				}
				else if (MetaFieldHasAttrib(mf, "SerializeFromId", &attrib)) {
					mf->flags = (FieldSerializeFlags)((int)mf->flags | FSF_SerializeFromId);
					mf->serializeFromId = attrib.args.data[0];
					mf->serializeExt = attrib.args.data[1];


					// Strip off quote marks
					mf->serializeFromId.start++;
					mf->serializeFromId.length -= 2;

					mf->serializeExt.start++;
					mf->serializeExt.length -= 2;
				}

				for (int k = 0; k < allParseMetaEnums.count; k++) {
					if (mf->type == allParseMetaEnums.data[k].name) {
						mf->flags = (FieldSerializeFlags)((int)mf->flags | FSF_SerializeAsEnum);
						mf->enumType = k;
						break;
					}
				}
			}
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
		"ParseQuaternion",
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

	fprintf(componentTypeFile, "enum CustomDefinedEnum{\n");
	for (int i = 0; i < allParseMetaEnums.count; i++) {
		ParseMetaEnum* me = &allParseMetaEnums.data[i];
		fprintf(componentTypeFile, "\tCDE_%.*s,\n", me->name.length, me->name.start);
	}
	fprintf(componentTypeFile, "\tCDE_Count\n");
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

	fprintf(componentResetFile, "void Scene::DestroyCustomComponentsByEntity(IDHandle<Entity> entId){\n");
	for (int i = 0; i < getComponentPathList.count; i++) {
		SubString compTypeName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentResetFile, "\tfor (int i = 0; i < %s.currentCount; i++){\n", getComponentPathList.data[i].string);
		fprintf(componentResetFile, "\t\tComponent* comp = &%s.vals[i];\n", getComponentPathList.data[i].string);
		fprintf(componentResetFile, "\t\tif (comp->entity == entId){\n");
		fprintf(componentResetFile, "\t\t\t%s.RemoveByIdNum(comp->id);\n", getComponentPathList.data[i].string);
		fprintf(componentResetFile, "\t\t\tbreak;\n");
		fprintf(componentResetFile, "\t\t}\n");
		fprintf(componentResetFile, "\t}\n");
	}
	fprintf(componentResetFile, "}\n");

	fprintf(componentResetFile, "\nvoid Scene::SendCollisionToCustomComponents(IDHandle<Entity> entityId, Collision col){\n");
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

	fprintf(componentMetaFile, "\n");

	for (int i = 0; i < additionalIncludes.count; i++) {
		fprintf(componentMetaFile, "#include \"../%s\"\n", additionalIncludes.Get(i).string);
	}

	fprintf(componentMetaFile, "\n");

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
		fprintf(componentMetaFile, "\treturn %s.GetByIdNum(id);\n", getComponentPathList.data[i].string);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "Component* %.*s_createAndAdd(){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t%.*s* comp = %s.CreateAndAdd();\n", ms->name.length, ms->name.start, getComponentPathList.data[i].string);
		fprintf(componentMetaFile, "\tcomp->type = CCT_%.*s;\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tcomp->type = CCT_%.*s;\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\treturn comp;\n");
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "void %.*s_Remove(uint32 id){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t%s.RemoveByIdNum(id);\n", getComponentPathList.data[i].string);
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

		int fieldCount = 0;
		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			SubString enumName;
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount, allParseMetaEnums, &enumName);
			if (fieldType != MT_Unknown) {
				fieldCount++;
			}
		}

		if (fieldCount > 0) {
			fprintf(componentMetaFile, "MetaField %.*s_metaFields[] = {\n", ms->name.length, ms->name.start);
			for (int j = 0; j < ms->fields.count; j++) {
				ParseMetaField mf = ms->fields.data[j];
				SubString enumName;
				MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount, allParseMetaEnums, &enumName);
				if (fieldType == MT_CustomEnum) {
					fprintf(componentMetaFile, "\t{\"%.*s\", (int)(size_t)(&((%.*s*)0)->%.*s), %d, \"%.*s\", (FieldSerializeFlags)%d},\n",
						mf.name.length, mf.name.start,
						ms->name.length, ms->name.start,
						mf.name.length, mf.name.start,
						mf.enumType,
						mf.serializeExt.length, mf.serializeExt.start,
						mf.flags);
				}
				else if (fieldType != MT_Unknown) {
					fprintf(componentMetaFile, "\t{\"%.*s\", (int)(size_t)(&((%.*s*)0)->%.*s), %d, \"%.*s\", (FieldSerializeFlags)%d},\n",
						mf.name.length, mf.name.start,
						ms->name.length, ms->name.start,
						mf.name.length, mf.name.start,
						(int)fieldType,
						mf.serializeExt.length, mf.serializeExt.start,
						mf.flags);
				}
			}
			fprintf(componentMetaFile, "};\n\n");
		}

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

		if (fieldCount > 0) {
			fprintf(componentMetaFile, "\t%.*s_metaFields,\n", ms->name.length, ms->name.start);
		}
		else {
			fprintf(componentMetaFile, "\tnullptr,\n");
		}
		fprintf(componentMetaFile, "\t%d,\n", fieldCount);
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
		fprintf(componentMetaFile, "\t(void)compCast;\n");
		fprintf(componentMetaFile, "\tString temp;\n");
		fprintf(componentMetaFile, "\t(void)temp;\n");

		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			SubString enumName;
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount, allParseMetaEnums, &enumName);

			if (fieldType == MT_CustomEnum) {
				fprintf(componentMetaFile, "\tif(elem->attributes.LookUp(\"%.*s\", &temp)){\n", mf.name.length, mf.name.start);
				fprintf(componentMetaFile, "\t\tcompCast->%.*s = (%.*s)ParseEnum(%d, temp.string);\n\t}\n",
					mf.name.length, mf.name.start, mf.type.length, mf.type.start, mf.enumType);
			}
			else if (fieldType == MT_Handle) {
				if (mf.flags & FSF_SerializeFromId) {
					fprintf(componentMetaFile, "\tif(elem->attributes.LookUp(\"%.*s\", &temp)){\n", mf.serializeFromId.length, mf.serializeFromId.start);
					fprintf(componentMetaFile, "\t\tGlobalScene->res.assetIdMap.LookUp(temp.string, (int*)&compCast->%.*s.id);\n\t}\n", mf.name.length, mf.name.start);
				}
				else {
					fprintf(componentMetaFile, "\tif(elem->attributes.LookUp(\"%.*s\", &temp)){\n", mf.name.length, mf.name.start);
					fprintf(componentMetaFile, "\t\tcompCast->%.*s.id = Atoi(temp.string);\n\t}\n", mf.name.length, mf.name.start);
				}
			}
			else if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				if (mf.flags & FSF_SerializeFromId) {
					fprintf(componentMetaFile, "\tif(elem->attributes.LookUp(\"%.*s\", &temp)){\n", mf.serializeFromId.length, mf.serializeFromId.start);
					fprintf(componentMetaFile, "\t\tGlobalScene->res.assetIdMap.LookUp(temp.string, &compCast->%.*s);\n\t}\n", mf.name.length, mf.name.start);
				}
				else {
					fprintf(componentMetaFile, "\tif(elem->attributes.LookUp(\"%.*s\", &temp)){\n", mf.name.length, mf.name.start);
					fprintf(componentMetaFile, "\t\tcompCast->%.*s = %s(temp.string);\n\t}\n", mf.name.length, mf.name.start, parseFuncs[(int)fieldType]);
				}
			}
		}

		fprintf(componentMetaFile, "}\n\n");
		fprintf(componentMetaFile, "void %.*s_XMLSerialize(const Component* comp, XMLElement* elem){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tconst %.*s* compCast = static_cast<const %.*s*>(comp);\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t(void)compCast;\n");
		fprintf(componentMetaFile, "\telem->name = STATIC_TO_SUBSTRING(\"%.*s\");\n", ms->name.length, ms->name.start);

		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			SubString enumName;
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount, allParseMetaEnums, &enumName);

			if (fieldType == MT_CustomEnum) {
				fprintf(componentMetaFile, "\telem->attributes.Insert(\"%.*s\", EncodeEnum(%d, compCast->%.*s));\n",
					mf.name.length, mf.name.start, mf.enumType, mf.name.length, mf.name.start);
			}
			else if (fieldType == MT_Handle) {
				if (mf.flags & FSF_SerializeFromId) {
					fprintf(componentMetaFile, "\tString id_%d = GlobalScene->res.FindFileNameByIdAndExtension(\"%.*s\", compCast->%.*s.id);\n",
						j, mf.serializeExt.length, mf.serializeExt.start, mf.name.length, mf.name.start);
					fprintf(componentMetaFile, "\telem->attributes.Insert(\"%.*s\", id_%d);\n", mf.serializeFromId.length, mf.serializeFromId.start, j);
				}
				else {
					fprintf(componentMetaFile, "\telem->attributes.Insert(\"%.*s\", Itoa(compCast->%.*s.id));\n", mf.name.length, mf.name.start, mf.name.length, mf.name.start);
				}
			}
			else if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				if (mf.flags & FSF_SerializeFromId) {
					fprintf(componentMetaFile, "\tString id_%d = GlobalScene->res.FindFileNameByIdAndExtension(\"%.*s\", compCast->%.*s);\n",
						j, mf.serializeExt.length, mf.serializeExt.start, mf.name.length, mf.name.start);
					fprintf(componentMetaFile, "\telem->attributes.Insert(\"%.*s\", id_%d);\n", mf.serializeFromId.length, mf.serializeFromId.start, j);
				}
				else {
					fprintf(componentMetaFile, "\telem->attributes.Insert(\"%.*s\", %s(compCast->%.*s));\n", mf.name.length, mf.name.start, encodeFuncs[(int)fieldType], mf.name.length, mf.name.start);
				}
			}
		}

		fprintf(componentMetaFile, "}\n\n");
		fprintf(componentMetaFile, "void %.*s_MemSerialize(const Component* comp, MemStream* stream){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tconst %.*s* compCast = static_cast<const %.*s*>(comp);\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t(void)compCast;\n");
		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			SubString enumName;
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount, allParseMetaEnums, &enumName);

			if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				if (fieldType == MT_Handle) {
					fprintf(componentMetaFile, "\tstream->Write<%.*s<%.*s>>(compCast->%.*s);\n", 
						mf.type.length, mf.type.start, mf.typeParam.length, mf.typeParam.start, mf.name.length, mf.name.start);
				}
				else {
					fprintf(componentMetaFile, "\tstream->Write<%.*s>(compCast->%.*s);\n", 
						mf.type.length, mf.type.start, mf.name.length, mf.name.start);
				}
			}
		}
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "void %.*s_MemDeserialize(Component* comp, MemStream* stream){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t%.*s* compCast = static_cast<%.*s*>(comp);\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t(void)compCast;\n");
		for (int j = 0; j < ms->fields.count; j++) {
			ParseMetaField mf = ms->fields.data[j];
			SubString enumName;
			MetaType fieldType = ParseType(mf.type, mf.indirectionLevel, mf.arrayCount, allParseMetaEnums, &enumName);

			if (fieldType != MT_Unknown && FindMetaAttribByName(mf.attrs, "DoNotSerialize") == nullptr) {
				if (fieldType == MT_Handle) {
					fprintf(componentMetaFile, "\tcompCast->%.*s = stream->Read<%.*s<%.*s>>();\n",
						mf.name.length, mf.name.start, mf.type.length, mf.type.start, mf.typeParam.length, mf.typeParam.start);
				}
				else {
					fprintf(componentMetaFile, "\tcompCast->%.*s = stream->Read<%.*s>();\n", 
						mf.name.length, mf.name.start, mf.type.length, mf.type.start);
				}
			}
		}
		fprintf(componentMetaFile, "}\n\n");

		const char* lvlPath = getComponentLevelPathList.Get(i).string;
		fprintf(componentMetaFile, "Component* %.*s_addToLevel(Level* lvl){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\tlvl->%s.PushBack(%.*s());\n", lvlPath, ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\treturn &lvl->%s.data[lvl->%s.count-1];\n", lvlPath, lvlPath);
		fprintf(componentMetaFile, "}\n\n");

		fprintf(componentMetaFile, "void %.*s_ResetComp(Component* comp){\n", ms->name.length, ms->name.start);
		fprintf(componentMetaFile, "\t((%.*s*)comp)->~%.*s();\n", ms->name.length, ms->name.start, ms->name.length, ms->name.start);
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

	fprintf(componentMetaFile, "GetComponentFunc* getComponentFuncs[CCT_Count] = {\n");
	for (int i = 0; i < componentIndices.count; i++) {
		SubString structName = allParseMetaStructs.Get(componentIndices.Get(i)).name;
		fprintf(componentMetaFile, "\t%.*s_getComponentById,\n", structName.length, structName.start);
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

	for (int i = 0; i < allParseMetaEnums.count; i++) {
		ParseMetaEnum* me = &allParseMetaEnums.data[i];

		fprintf(componentMetaFile, "MetaEnumEntry %.*s_entries[] = {\n", me->name.length, me->name.start);
		int entryCount = 0;
		for (int j = 0; j < me->entries.count; j++) {
			ParseMetaEnumEntry* mee = &me->entries.data[j];
			if (!EnumEntryHasAttrib(mee, "DoNotSerialize")) {
				SubString name = mee->name;
				fprintf(componentMetaFile, "\t{\"%.*s\", \"%.*s\", ", name.length, name.start, name.length, name.start);
				for (int k = 0; k < mee->value.count; k++) {
					fprintf(componentMetaFile, "%.*s", mee->value.data[k].length, mee->value.data[k].start);
				}
				fprintf(componentMetaFile, "},\n");
				entryCount++;
			}
		}
		fprintf(componentMetaFile, "};\n");

		fprintf(componentMetaFile, "MetaEnum %.*s_meta = {\"%.*s\", %.*s_entries, %d, %d, %d, (MetaEnumFlags)%d};\n\n", 
			me->name.length, me->name.start, 
			me->name.length, me->name.start,
			me->name.length, me->name.start,
			entryCount,
			me->allIdx,
			me->noneIdex,
			me->flags);
	}
	
	fprintf(componentMetaFile, "MetaEnum* enumMetaData[CDE_Count] = {\n");
	for (int i = 0; i < allParseMetaEnums.count; i++) {
		fprintf(componentMetaFile, "\t&%.*s_meta,\n", allParseMetaEnums.Get(i).name.length, allParseMetaEnums.Get(i).name.start);
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

	// HACK: This makes the whole thing less automatic.
	// TODO: Either restructure this to make it unnecessary, or automate this.
	const char* structForwardDecls[] = {
		"Editor"
	};

	for (int i = 0; i < BNS_ARRAY_COUNT(structForwardDecls); i++) {
		fprintf(actionHeaderFile, "struct %s;\n", structForwardDecls[i]);
	}

	for (int i = 0; i < funcsWithActionAttrib.count; i++) {
		DumpFunctionHeader(&funcsWithActionAttrib.data[i], actionHeaderFile);
		fprintf(actionHeaderFile, "\n");
	}

	fprintf(actionHeaderFile, "\n");
	fprintf(actionHeaderFile, "enum ActionType {\n");
	fprintf(actionHeaderFile, "\tAT_None = -1,\n");
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

	fprintf(actionHeaderFile, "\tAction(){\n");
	fprintf(actionHeaderFile, "\t\ttype = AT_None;\n");
	fprintf(actionHeaderFile, "\t}\n");

	fprintf(actionHeaderFile, "\tAction(const Action& orig){\n");
	fprintf(actionHeaderFile, "\t\ttype = orig.type;\n");
	fprintf(actionHeaderFile, "\t\tswitch (type){\n");
	fprintf(actionHeaderFile, "\t\tcase AT_None: {} break;\n");
	for (int i = 0; i < funcsWithActionAttrib.count; i++) {
		ParseMetaFuncDef* def = &funcsWithActionAttrib.data[i];
		fprintf(actionHeaderFile, "\t\tcase AT_%.*s: {\n", def->name.length, def->name.start);
		fprintf(actionHeaderFile, "\t\t\t%.*s_data = orig.%.*s_data;\n", def->name.length, def->name.start, def->name.length, def->name.start);
		fprintf(actionHeaderFile, "\t\t} break;\n");
	}
	fprintf(actionHeaderFile, "\t\tdefault:{ASSERT_WARN(\"%%s\", \"Incorrect value for action!\");break;}\n");
	fprintf(actionHeaderFile, "\t\t}\n");
	fprintf(actionHeaderFile, "\t}\n");

	fprintf(actionHeaderFile, "};\n\n");

	fprintf(actionHeaderFile, "inline void ExecuteAction(const Action action){\n");
	fprintf(actionHeaderFile, "\tswitch (action.type){\n");
	fprintf(actionHeaderFile, "\tcase AT_None: {} break;\n");

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

	//-------------------------
	// Script funcs

	FILE* scriptGenHeaderFile = fopen("gen/ScriptGen.h", "wb");
	FILE* scriptGenSourceFile = fopen("gen/ScriptGen.cpp", "wb");
	FILE* scriptGenIncludeFile = fopen("assets/scripts/gen/extern.bni", "wb");

	fprintf(scriptGenSourceFile, "#include \"../ext/CppUtils/bnvm.h\"\n");
	fprintf(scriptGenSourceFile, "#include \"../ext/3dbasics/Vector3.h\"\n\n");

	fprintf(scriptGenHeaderFile, "#ifndef SCRIPTGEN_H\n");
	fprintf(scriptGenHeaderFile, "#define SCRIPTGEN_H\n\n");

	fprintf(scriptGenHeaderFile, "#pragma once\n\n");

	fprintf(scriptGenHeaderFile, "void RegisterBNGineVM(BNVM* vm);\n\n");

	Vector<ParseMetaFuncDef> scriptFuncs;
	for (int i = 0; i < allParseMetaFuncs.count; i++) {
		if (FuncDefHasAttrib(&allParseMetaFuncs.data[i], "ScriptFunc")) {
			scriptFuncs.PushBack(allParseMetaFuncs.data[i]);
		}
	}

	for (int i = 0; i < scriptFuncs.count; i++) {
		ParseMetaFuncDef def = scriptFuncs.data[i];

		fprintf(scriptGenIncludeFile, "extern %.*s %.*s(", def.retType.typeName.length, def.retType.typeName.start, def.name.length, def.name.start);
		for (int j = 0; j < def.params.count; j++) {
			MetaVarDecl param = def.params.data[j];
			fprintf(scriptGenIncludeFile, "%s%.*s %.*s", (j > 0 ? ", " : ""), 
				param.type.typeName.length, param.type.typeName.start, param.name.length, param.name.start);
		}
		fprintf(scriptGenIncludeFile, ");\n");


		fprintf(scriptGenHeaderFile, "void %.*s_Script(TempStack* stk);\n", def.name.length, def.name.start);

		DumpFunctionHeader(&def, scriptGenSourceFile);

		fprintf(scriptGenSourceFile, "\nvoid %.*s_Script(TempStack* stk){\n", def.name.length, def.name.start);
		for (int j = 0; j < def.params.count; j++) {
			MetaVarDecl decl = def.params.data[j];
			decl.type.isConst = false;
			decl.type.isReference = false;

			fprintf(scriptGenSourceFile, "\t%.*s %.*s = stk->Pop<%.*s>();\n",
				decl.type.typeName.length, decl.type.typeName.start,
				decl.name.length, decl.name.start,
				decl.type.typeName.length, decl.type.typeName.start);
		}

		fprintf(scriptGenSourceFile, "\t");
		if (def.retType.typeName != "void") {
			fprintf(scriptGenSourceFile, "%.*s _ret = ", def.retType.typeName.length, def.retType.typeName.start);
		}

		fprintf(scriptGenSourceFile, "%.*s(", def.name.length, def.name.start);
		for (int j = 0; j < def.params.count; j++) {
			SubString paramName = def.params.data[j].name;
			fprintf(scriptGenSourceFile, "%s%.*s", (j > 0 ? ", " : ""), paramName.length, paramName.start);
		}
		fprintf(scriptGenSourceFile, ");\n");

		if (def.retType.typeName != "void") {
			fprintf(scriptGenSourceFile, "\tstk->Push<%.*s>(_ret);\n", def.retType.typeName.length, def.retType.typeName.start);
		}

		fprintf(scriptGenSourceFile, "}\n\n");
	}

	fprintf(scriptGenHeaderFile, "#endif\n");

	fprintf(scriptGenSourceFile, "\nvoid RegisterBNGineVM(BNVM* vm){\n");
	for (int i = 0; i < scriptFuncs.count; i++) {
		ParseMetaFuncDef def = scriptFuncs.data[i];
		fprintf(scriptGenSourceFile, "\tvm->RegisterExternFunc(\"%.*s\", %.*s_Script);\n", def.name.length, def.name.start, def.name.length, def.name.start);
	}
	fprintf(scriptGenSourceFile, "}\n\n");

	fclose(scriptGenIncludeFile);
	fclose(scriptGenSourceFile);
	fclose(scriptGenHeaderFile);

	//-------------------------
}



