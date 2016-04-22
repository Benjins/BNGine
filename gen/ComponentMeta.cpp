#include "../src/core/Scene.h"
#include "../src/core/Component.h"
#include "../src/metagen/MetaStruct.h"
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
	elem->attributes.LookUp("position", &temp);
	compCast->position = ParseVector3(temp.string);
	elem->attributes.LookUp("size", &temp);
	compCast->size = ParseVector3(temp.string);
}

void BoxCollider_XMLSerialize(const Component* comp, XMLElement* elem){
	const BoxCollider* compCast = static_cast<const BoxCollider*>(comp);
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

void SphereCollider_XMLDeserialize(Component* comp, const XMLElement* elem){
	SphereCollider* compCast = static_cast<SphereCollider*>(comp);
	String temp;
	elem->attributes.LookUp("position", &temp);
	compCast->position = ParseVector3(temp.string);
	elem->attributes.LookUp("radius", &temp);
	compCast->radius = atof(temp.string);
}

void SphereCollider_XMLSerialize(const Component* comp, XMLElement* elem){
	const SphereCollider* compCast = static_cast<const SphereCollider*>(comp);
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

