#ifndef COMPONENTMETA_H
#define COMPONENTMETA_H

#pragma once

#include "../../gen/ComponentType.h"
#include "../core/Component.h"

struct XMLElement;
struct MemStream;

typedef Component* (GetComponentFunc)(uint32 id);
typedef Component* (AddComponentFunc)(uint32 id);

extern GetComponentFunc* getComponentFuncs[CT_Count];
extern AddComponentFunc* addComponentFuncs[CT_Count];

typedef void (ComponentXMLSerializeFunc)(Component*, const XMLElement*);
typedef void (ComponentXMLDeserializeFunc)(const Component*,XMLElement*);
typedef void (ComponentMemSerializeFunc)(Component*, const MemStream*);
typedef void (ComponentMemDeserializeFunc)(const Component*, MemStream*);

extern ComponentXMLDeserializeFunc* componentXMLDeserializeFuncs[CT_Count];
extern ComponentXMLSerializeFunc* componentXMLSerializeFuncs[CT_Count];
extern ComponentMemDeserializeFunc* componentMemDeserializeFuncs[CT_Count];
extern ComponentMemSerializeFunc* componentMemSerializeFuncs[CT_Count];



#endif
