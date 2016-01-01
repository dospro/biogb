#ifndef BIOGB_MACROS
#define BIOGB_MACROS

using u8 = unsigned char;
using u16 = unsigned short;
using s8 = signed char;
using s16 = short;
using  u32 = unsigned int;
using s32 = int;

#define F_Z 128
#define F_N 64
#define F_H 32
#define F_C 16

#ifdef DEBUG_MESSAGES
	#define ERROR(condicion,mensaje)if(condicion)printf("ERROR: %s\n", mensaje);
	#define WARNING(condicion,mensaje)if(condicion)printf("WARNING: %s\n", mensaje);
	#define LOG(mensaje)printf("%d\n", mensaje);
#else
	#define ERROR(condicion,mensaje)
	#define WARNING(condicion,mensaje)
	#define LOG(mensaje)
#endif

#endif

