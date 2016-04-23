#ifndef COMPONENTMETA_H
#define COMPONENTMETA_H

#pragma once

#include "../../gen/ComponentType.h"
#include "../core/Component.h"

struct XMLElement;
struct MemStream;
struct MetaStruct;
struct Level;

typedef Component* (GetComponentFunc)(uint32 id);
typedef Component* (GetComponentArrayFunc)();
typedef Component* (AddComponentFunc)();
typedef int (GetComponentCountFunc)();

extern GetComponentFunc* getComponentFuncs[CCT_Count];
extern GetComponentArrayFunc* getComponentArrayFuncs[CCT_Count];
extern AddComponentFunc* addComponentFuncs[CCT_Count];
extern GetComponentCountFunc* getComponentCountFuncs[CCT_Count];

typedef Component* (AddComponentToLevelFunc)(Level* lvl);
extern AddComponentToLevelFunc* addComponentToLevelFuncs[CCT_Count];

typedef void (ComponentXMLDeserializeFunc)(Component*, const XMLElement*);
typedef void (ComponentXMLSerializeFunc)(const Component*,XMLElement*);
typedef void (ComponentMemDeserializeFunc)(Component*, MemStream*);
typedef void (ComponentMemSerializeFunc)(const Component*, MemStream*);

extern ComponentXMLDeserializeFunc* componentXMLDeserializeFuncs[CCT_Count];
extern ComponentXMLSerializeFunc* componentXMLSerializeFuncs[CCT_Count];
extern ComponentMemDeserializeFunc* componentMemDeserializeFuncs[CCT_Count];
extern ComponentMemSerializeFunc* componentMemSerializeFuncs[CCT_Count];

extern MetaStruct* componentMetaData[CCT_Count];

int FindStructByName(const char* name);

#endif
