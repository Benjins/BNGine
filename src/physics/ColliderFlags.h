#ifndef COLLIDEERFLAGS_H
#define COLLIDEERFLAGS_H

#pragma once

#define COLLIDERLAYER_MASK 0xFE

/*[Flags]*/
enum ColliderFlags {
	/*[NoneFlag]*/
	CF_NONE = 0,
	CF_STATIC = (1 << 0),
	CF_TRIGGER = (1 << 1),
	/*[AllFlag]*/
	CF_ALL = (1 << 2) - 1
};


#endif
