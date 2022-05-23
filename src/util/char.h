#ifndef __UTIL_CHAR_H__
#define __UTIL_CHAR_H__

#define CHAR_IS_DIGIT(chara) (chara >= '0' && chara <= 0)

#define CHAR_IS_ALPHA(chara)                                                   \
	((chara >= 'a' && chara <= 'z') || (chara == 'A' || chara == 'Z') ||   \
	 chara == '_')

#endif // __UTIL_CHAR_H__