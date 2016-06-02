#include "../src/core/Scene.h"
#include "../src/core/Component.h"
#include "../src/metagen/MetaStruct.h"
#include "../src/metagen/ComponentMeta.h"
#include "../src/util/Serialization.h"

#include "../ext/CppUtils/xml.h"
#include "../ext/CppUtils/memstream.h"
#include "ComponentType.h"

#include "../src/physics/BoxCollider.h"//BoxCollider
#include "../src/physics/SphereCollider.h"//SphereCollider
Component* BoxCollider_getComponentArray(){
	return GlobalScene->phys.boxCols.vals;
}

Component* BoxCollider_getComponentById(uint32 id){
	return GlobalScene->phys.boxCols.GetById(id);
}

Component* BoxCollider_createAndAdd(){
	Component* comp = GlobalScene->phys.boxCols.CreateAndAdd();
	comp->type = CCT_BoxCollider;
	return comp;
}

Component* BoxCollider_getLevelArray(const Level* lvl){
	Component* comps = lvl->boxCols.data;
	return comps;
}

int BoxCollider_getLevelCount(const Level* lvl){
	int count = lvl->boxCols.count;
	return count;
}

Component* SphereCollider_getComponentArray(){
	return GlobalScene->phys.sphereCols.vals;
}

Component* SphereCollider_getComponentById(uint32 id){
	return GlobalScene->phys.sphereCols.GetById(id);
}

Component* SphereCollider_createAndAdd(){
	Component* comp = GlobalScene->phys.sphereCols.CreateAndAdd();
	comp->type = CCT_SphereCollider;
	return comp;
}

Component* SphereCollider_getLevelArray(const Level* lvl){
	Component* comps = lvl->sphereCols.data;
	return comps;
}

int SphereCollider_getLevelCount(const Level* lvl){
	int count = lvl->sphereCols.count;
	return count;
}

MetaField BoxCollider_metaFields[] = {
	{"position", (int)(size_t)(&((BoxCollider*)0)->position), (MetaType)6},
	{"size", (int)(size_t)(&((BoxCollider*)0)->size), (MetaType)6},
};

MetaStruct BoxCollider_meta = {
	"BoxCollider",
	nullptr,
	BoxCollider_metaFields,
	2,
	sizeof(BoxCollider)
};

MetaField SphereCollider_metaFields[] = {
	{"position", (int)(size_t)(&((SphereCollider*)0)->position), (MetaType)6},
	{"radius", (int)(size_t)(&((SphereCollider*)0)->radius), (MetaType)1},
};

MetaStruct SphereCollider_meta = {
	"SphereCollider",
	nullptr,
	SphereCollider_metaFields,
	2,
	sizeof(SphereCollider)
};

void BoxCollider_XMLDeserialize(Component* comp, const XMLElement* elem){
	BoxCollider* compCast = static_cast<BoxCollider*>(comp);
	String temp;
	if(elem->attributes.LookUp("position", &temp)){
		compCast->position = ParseVector3(temp.string);
	}
	if(elem->attributes.LookUp("size", &temp)){
		compCast->size = ParseVector3(temp.string);
	}
}

void BoxCollider_XMLSerialize(const Component* comp, XMLElement* elem){
	const BoxCollider* compCast = static_cast<const BoxCollider*>(comp);
	elem->name = STATIC_TO_SUBSTRING("BoxCollider");
	elem->attributes.Insert("position", EncodeVector3(compCast->position));
	elem->attributes.Insert("size", EncodeVector3(compCast->size));
}

void BoxCollider_MemSerialize(const Component* comp, MemStream* stream){
	const BoxCollider* compCast = static_cast<const BoxCollider*>(comp);
	stream->Write<Vector3>(compCast->position);
	stream->Write<Vector3>(compCast->size);
}

void BoxCollider_MemDeserialize(Component* comp, MemStream* stream){
	BoxCollider* compCast = static_cast<BoxCollider*>(comp);
	compCast->position = stream->Read<Vector3>();
	compCast->size = stream->Read<Vector3>();
}

Component* BoxCollider_addToLevel(Level* lvl){
	
lvl->boxCols.EnsureCapacity(lvl->boxCols.count+1);
	lvl->boxCols.count++;
	return &lvl->boxCols.data[lvl->boxCols.count-1];
}

void SphereCollider_XMLDeserialize(Component* comp, const XMLElement* elem){
	SphereCollider* compCast = static_cast<SphereCollider*>(comp);
	String temp;
	if(elem->attributes.LookUp("position", &temp)){
		compCast->position = ParseVector3(temp.string);
	}
	if(elem->attributes.LookUp("radius", &temp)){
		compCast->radius = atof(temp.string);
	}
}

void SphereCollider_XMLSerialize(const Component* comp, XMLElement* elem){
	const SphereCollider* compCast = static_cast<const SphereCollider*>(comp);
	elem->name = STATIC_TO_SUBSTRING("SphereCollider");
	elem->attributes.Insert("position", EncodeVector3(compCast->position));
	elem->attributes.Insert("radius", Ftoa(compCast->radius));
}

void SphereCollider_MemSerialize(const Component* comp, MemStream* stream){
	const SphereCollider* compCast = static_cast<const SphereCollider*>(comp);
	stream->Write<Vector3>(compCast->position);
	stream->Write<float>(compCast->radius);
}

void SphereCollider_MemDeserialize(Component* comp, MemStream* stream){
	SphereCollider* compCast = static_cast<SphereCollider*>(comp);
	compCast->position = stream->Read<Vector3>();
	compCast->radius = stream->Read<float>();
}

Component* SphereCollider_addToLevel(Level* lvl){
	
lvl->sphereCols.EnsureCapacity(lvl->sphereCols.count+1);
	lvl->sphereCols.count++;
	return &lvl->sphereCols.data[lvl->sphereCols.count-1];
}

AddComponentToLevelFunc* addComponentToLevelFuncs[CCT_Count] = {
	BoxCollider_addToLevel,
	SphereCollider_addToLevel,
};
AddComponentFunc* addComponentFuncs[CCT_Count] = {
	BoxCollider_createAndAdd,
	SphereCollider_createAndAdd,
};
ComponentXMLDeserializeFunc* componentXMLDeserializeFuncs[CCT_Count] = {
	BoxCollider_XMLDeserialize,
	SphereCollider_XMLDeserialize,
};
ComponentXMLSerializeFunc* componentXMLSerializeFuncs[CCT_Count] = {
	BoxCollider_XMLSerialize,
	SphereCollider_XMLSerialize,
};
ComponentMemDeserializeFunc* componentMemDeserializeFuncs[CCT_Count] = {
	BoxCollider_MemDeserialize,
	SphereCollider_MemDeserialize,
};
ComponentMemSerializeFunc* componentMemSerializeFuncs[CCT_Count] = {
	BoxCollider_MemSerialize,
	SphereCollider_MemSerialize,
};
GetComponentLevelArrayFunc* getComponentLevelArrayFuncs[CCT_Count] = {
	BoxCollider_getLevelArray,
	SphereCollider_getLevelArray,
};
GetComponentLevelCountFunc* getComponentLevelCountFuncs[CCT_Count] = {
	BoxCollider_getLevelCount,
	SphereCollider_getLevelCount,
};
MetaStruct* componentMetaData[CCT_Count] = {
	&BoxCollider_meta,
	&SphereCollider_meta,
};
