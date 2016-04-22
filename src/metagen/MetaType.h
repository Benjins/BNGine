#ifndef METATYPE_H
#define METATYPE_H

#pragma once

enum MetaType {
	MT_Unknown = -1,
	MT_FundamentalBegin = 0,
	MT_Int = MT_FundamentalBegin,
	MT_Float,
	MT_String,
	MT_StringRef,
	MT_SubStringRef,
	MT_Vector2,
	MT_Vector3,
	MT_Vector4,
	MT_Quaternion,
	MT_FundamentalEnd
};

#endif
