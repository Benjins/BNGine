#ifndef METATYPE_H
#define METATYPE_H

#pragma once

enum FundamentalType {
	FT_Unknown = -1,
	FT_Begin = 0,
	FT_Int = FT_Begin,
	FT_Float,
	FT_String,
	FT_Vector2,
	FT_Vector3,
	FT_Vector4,
	FT_Quaternion,
	FT_FundamentalEnd
};

#endif
