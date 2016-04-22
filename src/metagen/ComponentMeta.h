#ifndef COMPONENTMETA_H
#define COMPONENTMETA_H

#pragma once

#include "../../gen/ComponentType.h"
#include "../core/Component.h"

struct XMLElement;
struct MemStream;

typedef Component* (GetComponentFunc)(uint32 id);
typedef Component* (GetComponentArrayFunc)();
typedef Component* (AddComponentFunc)(uint32 id);

extern GetComponentFunc* getComponentFuncs[CT_Count];
extern GetComponentArrayFunc* getComponentArrayFuncs[CT_Count];
extern AddComponentFunc* addComponentFuncs[CT_Count];

typedef void (ComponentXMLDeserializeFunc)(Component*, const XMLElement*);
typedef void (ComponentXMLSerializeFunc)(const Component*,XMLElement*);
typedef void (ComponentMemDeserializeFunc)(Component*, MemStream*);
typedef void (ComponentMemSerializeFunc)(const Component*, MemStream*);

extern ComponentXMLDeserializeFunc* componentXMLDeserializeFuncs[CT_Count];
extern ComponentXMLSerializeFunc* componentXMLSerializeFuncs[CT_Count];
extern ComponentMemDeserializeFunc* componentMemDeserializeFuncs[CT_Count];
extern ComponentMemSerializeFunc* componentMemSerializeFuncs[CT_Count];

extern MetaStruct componentMetaData[CT_Count];

#endif
