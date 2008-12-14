#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#ifndef NGC
#include "rc.h"
#endif
#include "fb.h"
#ifdef USE_ASM
#include "asm.h"
#endif

struct lcd lcd;

struct scan scan;

#define BG (scan.bg)
#define WND (scan.wnd)
#define BUF (scan.buf)
#define PRI (scan.pri)

#define PAL1 (scan.pal1)
#define PAL2 (scan.pal2)
#define PAL4 (scan.pal4)

#define VS (scan.vs) /* vissprites */
#define NS (scan.ns)

#define L (scan.l) /* line */
#define X (scan.x) /* screen position */
#define Y (scan.y)
#define S (scan.s) /* tilemap position */
#define T (scan.t)
#define U (scan.u) /* position within tile */
#define V (scan.v)
#define WX (scan.wx)
#define WY (scan.wy)
#define WT (scan.wt)
#define WV (scan.wv)

byte patpix[4096][8][8];
byte patdirty[1024];
byte anydirty;

#ifndef NGC
static int scale;
static int density;
static int rgb332;
static int usefilter, filterdmg;
static int sprsort = 1;
static int sprdebug;
static int paletteindex;
#else
int scale = 3;
int density = 3;
u8 rgb332 = 0;
u8 usefilter = 1;
u8 filterdmg = 0;
u8 sprsort = 1;
u8 sprdebug = 0;
u8 paletteindex = 0;
#endif // NGC Configurable Parameters

//Color: 0xBBGGRR
#define DEF_PAL { 0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C }
/*static int dmg_pal[4][4] = { 
       DEF_PAL, DEF_PAL, DEF_PAL, DEF_PAL 
};*/
//Default pallete
static int dmg_pal[28][4][4] = {
    {//Default Pallete
     {   0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C },
     {   0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C },
     {   0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C },
     {   0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C }
    },
    {//Grey Pallete
     {  0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }, //BG
     {	0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }, //WIN
     {	0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }, //OB0
     {	0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }  //OB1
    },
    {//Pallete of GoombaColor -Multi1 -Corrected
     {	0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }, //BG
     {	0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }, //WIN
     {	0xFFEFEF, 0xE78C5A, 0x9C4A18, 0x000000 }, //OB0
     {	0xEFEFFF, 0x635AEF, 0x1810AD, 0x000000 }  //OB1
    },
    {//Pallete of GoombaColor -Multi2 -Corrected
     {	0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }, //BG
     {	0xD6EFE7, 0x8CDEC6, 0x29846B, 0x000000 }, //WIN
     {	0xFFFFFF, 0x635AEF, 0x1810AD, 0x000000 }, //OB0
     {	0xFFFFFF, 0xE78C5A, 0x9C4A18, 0x000000 }  //OB1
    },
    {//Pallete of GoombaColor -Zelda -Corrected
     {	0xA0FFFF, 0x67D767, 0x20558C, 0x071546 }, //BG
     {	0xADFFFF, 0xB78080, 0x592925, 0x000000 }, //WIN
     {	0x7FB0FF, 0xE78C5A, 0x9C4A18, 0x000000 }, //OB0
     {	0xEFEFFF, 0x635AEF, 0x1810AD, 0x000000 }  //OB1
    },
    {//Pallete of GoombaColor -Metroid -Corrected
     {	0x7FDFDF, 0xAF5F00, 0x1F3F1F, 0x000000 }, //BG
     {	0x00DFFF, 0xFFFF80, 0x808000, 0x000000 }, //WIN
     {	0x00DFFF, 0x0000FF, 0x00373F, 0x000000 }, //OB0
     {	0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000 }  //OB1
    },
    {//Pallete of GoombaColor -Adventure Island -Corrected
     {	0xFFFFFF, 0xFFB59C, 0x009431, 0x000000 }, //BG
     {	0xFFFFFF, 0xFFB098, 0x009431, 0x000000 }, //WIN
     {	0xDEFFFF, 0x73C6EF, 0x5263FF, 0x290000 }, //OB0
     {	0xFFFFFF, 0xA5A5E7, 0x29297B, 0x000042 }  //OB1
    },
    {//Pallete of GoombaColor -Adventure Island 2 -Corrected
     {	0xFFFFFF, 0x75EFF7, 0xBD6B29, 0x000000 }, //BG
     {	0xFFFFFF, 0xFFB098, 0x009431, 0x000000 }, //WIN
     {	0xDEFFFF, 0x73C6EF, 0x5263FF, 0x290000 }, //OB0
     {	0xFFFFFF, 0xA5A5E7, 0x29297B, 0x000042 }  //OB1
    },
    {//Pallete of GoombaColor -Ballon Kid -Corrected
     {	0xFFD6A5, 0xFFEFE7, 0x108CDE, 0x00105A }, //BG
     {	0xFFFFFF, 0x73CEF7, 0x084A8C, 0x9C2121 }, //WIN
     {	0xC6C6FF, 0x6B6BFF, 0x0000FF, 0x000063 }, //OB0
     {	0xFFFFFF, 0xEF42EF, 0x29297B, 0x000042 }  //OB1
    },
    {//Pallete of GoombaColor -Batman -Corrected
     {	0xEFF7FF, 0x8890C8, 0x445084, 0x001042 }, //BG
     {	0xFFFFFF, 0xFFA5A5, 0xBD5252, 0xA50000 }, //WIN
     {	0xFFFFFF, 0xC6A5A5, 0x8C5252, 0x5A0000 }, //OB0
     {	0xFFFFFF, 0xBDB5AD, 0x7B6B5A, 0x422108 }  //OB1
    },
    {//Pallete of GoombaColor -Batman: Return of the Joker -Corrected
     {	0xFFFFFF, 0xBDADA5, 0x7B5A52, 0x391000 }, //BG
     {	0xFFFFFF, 0xBDADA5, 0x7B5A52, 0x391000 }, //WIN
     {	0xFFFFFF, 0xBDADA5, 0x7B5A52, 0x391000 }, //OB0
     {	0xFFFFFF, 0xBDB5AD, 0x7B6B5A, 0x422108 }  //OB1
    }, 
    {  //Pallete of GoombaColor -Bionic Commando -Corrected
     {	0xFFF7EF, 0xADB5CE, 0x2921C6, 0x000039}, //BG
     {	0xFFFFFF, 0x94CEF7, 0x1039FF, 0x00004A }, //WIN
     {	0xFFFFFF, 0x84ADFF, 0x00395A, 0x000000}, //OB0
     {	0xEFEFEF, 0x9CA5AD, 0x5A5A6B, 0x081042}  //OB1
    },
    {//Pallete of GoombaColor -Castlevania Adventure -Corrected
     {   0xE7D6D6, 0xB5A58C, 0x6B5242, 0x181000}, //BG
     {   0xFFFFFF, 0xD6A5A5, 0xAD5252, 0x840000}, //WIN
     {   0xFFFFFF, 0x84E7FF, 0x4252FF, 0x00005A}, //OB0
     {   0xFFFFFF, 0xCEEFF7, 0x9CDEF7, 0x6BB5F7}  //OB1
    },  
    {//Pallete of GoombaColor -Dr. Mario -Corrected
     {	0xFFFFFF, 0x66FFFF, 0xFF4221, 0x521000}, //BG
     {	0xFFFFFF, 0xD6AAAA, 0xAD5555, 0x840000}, //WIN
     {	0xFFFFFF, 0x84E7FF, 0x4252FF, 0x00008C}, //OB0
     {	0xFFFFFF, 0x8CCEFF, 0x5A9CF7, 0x005284}  //OB1
    },
    {//Pallete of GoombaColor -Kirby -Corrected
     {	0x83FFFF, 0x3EA5FF, 0x004273, 0x000933 }, //BG
     {	0xCCFFFF, 0x7778FF, 0x2335C1, 0x050A5A }, //WIN
     {	0xC4BDFF, 0x604DF1, 0x29129F, 0x000020 }, //OB0
     {	0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 }  //OB1
    },
    {//Pallete of GoombaColor -Donkey Kong Land -Corrected
     {	0xD1FFE2, 0x89FFA9, 0x48A251, 0x032504 }, //BG
     {	0xB4B4FF, 0x4747FF, 0x000080, 0x000000 }, //WIN
     {	0xB0F2FF, 0x4DC3D6, 0x115BA3, 0x00006A }, //OB0
     {	0x63FFF7, 0x42CEC6, 0x217B73, 0x000000 }  //OB1
    },
    {//Palette Wario Land -Corrected
     {	0xFFFFFF, 0xFFB59C, 0x4A7B94, 0x000000 }, //BG
     {	0xFFFFFF, 0xFFB098, 0x4A7B94, 0x000000 }, //WIN
     {	0xDEFFFF, 0x73C6EF, 0x5263FF, 0x290000 }, //OB0
     {	0xFFFFFF, 0xA5A5E7, 0x29297B, 0x000042 }  //OB1
    },

    /*{//Pallette of Gnuboy -R-Type
     {   0xC0FFFF, 0x408080, 0x204040, 0x000000 }, //BG
     {   0xC0FFFF, 0x408080, 0x204040, 0x000000 }, //WIN
     {   0xC0FFFF, 0x408080, 0x204040, 0x000000 }, //OB1
     {   0xC0FFFF, 0x408080, 0x204040, 0x000000 }  //OB2
    },*/
    {//Left
     {   0xFFFFFF, 0xF8A878, 0xF8A878, 0x000000 }, 
     {   0xFFFFFF, 0xF83000, 0xF83000, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0x004080, 0x000000 }
    },
    {//Left+A
     {   0xFFFFFF, 0xF6939D, 0xA03346, 0x000000 }, 
     {   0xFFFFFF, 0xF6939D, 0xA03346, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0x004080, 0x000000 } 
    },
    {//Up
     {   0xFFFFFF, 0x5098E8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0x5098E8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0x5098E8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0x5098E8, 0x004080, 0x000000 } 
    },
    {//Up+A
     {   0xFFFFFF, 0x8888E8, 0x2727A8, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0x2727A8, 0x000000 }, 
     {   0xFFFFFF, 0x00F800, 0xFF3300, 0x000000 }, 
     {   0xFFFFFF, 0x00F800, 0xFF3300, 0x000000 } 
    },
    {//Up+B
     {   0xFFFFFF, 0x94ACC0, 0x4A7B94, 0x000000 }, 
     {   0xFFFFFF, 0x94ACC0, 0x4A7B94, 0x000000 }, 
     {   0xFFFFFF, 0x5098E8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0x5098E8, 0x004080, 0x000000 }
    },
    {//Right
     {   0xFFFFFF, 0x00F800, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0x00F800, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0x00F800, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0x00F800, 0x0033F8, 0x000000 }
    },
    {//Right+A
     {   0xFFFFFF, 0x00F800, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0x00F800, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0xE88888, 0x2727A8, 0x000000 }, 
     {   0xFFFFFF, 0xE88888, 0x2727A8, 0x000000 } 
    },
    {//Right+B
     {   0x000000, 0xA1A200, 0x00FFF0, 0xFFFFFF },
     {   0x000000, 0xA1A200, 0x00FFF0, 0xFFFFFF },
     {   0x000000, 0xA1A200, 0x00FFF0, 0xFFFFFF },
     {   0x000000, 0xA1A200, 0x00FFF0, 0xFFFFFF }
    },
    {//Down
     {   0xFFFFFF, 0x8888E8, 0xF6939D, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0xF6939D, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0xF6939D, 0x000000 }, 
     {   0xFFFFFF, 0x8888E8, 0xF6939D, 0x000000 }
    },
    {//Down+A
     {   0xFFFFFF, 0x00FFF0, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0x00FFF0, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0x00FFF0, 0x0033F8, 0x000000 }, 
     {   0xFFFFFF, 0x00FFF0, 0x0033F8, 0x000000 } 
    },
    {//Down+B
     {   0xFFFFFF, 0x00FFF8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0x00FFF8, 0x004080, 0x000000 }, 
     {   0xFFFFFF, 0xF8A878, 0x008848, 0x000000 }, 
     {   0xFFFFFF, 0xF8A878, 0x008848, 0x000000 }
    }
};

static int filter[3][4] = {
	{ 195,  25,   0,  35 },
	{  25, 170,  25,  35 },
	{  25,  60, 125,  40 }
};

#ifndef NGC
rcvar_t lcd_exports[] =
{
	RCV_INT("scale", &scale),
	RCV_INT("density", &density),
	RCV_BOOL("rgb332", &rgb332),
	RCV_VECTOR("dmg_bgp", dmg_pal[paletteindex][0], 4),
	RCV_VECTOR("dmg_wndp", dmg_pal[paletteindex][1], 4),
	RCV_VECTOR("dmg_obp0", dmg_pal[paletteindex][2], 4),
	RCV_VECTOR("dmg_obp1", dmg_pal[paletteindex][3], 4),
	RCV_BOOL("sprsort", &sprsort),
	RCV_BOOL("sprdebug", &sprdebug),
	RCV_BOOL("colorfilter", &usefilter),
	RCV_BOOL("filterdmg", &filterdmg),
	RCV_VECTOR("red", filter[0], 4),
	RCV_VECTOR("green", filter[1], 4),
	RCV_VECTOR("blue", filter[2], 4),
	RCV_END
};
#endif

static byte *vdest;

#ifdef ALLOW_UNALIGNED_IO /* long long is ok since this is i386-only anyway? */
#define MEMCPY8(d, s) ((*(long long *)(d)) = (*(long long *)(s)))
#else
#define MEMCPY8(d, s) memcpy((d), (s), 8)
#endif




#ifndef ASM_UPDATEPATPIX
void updatepatpix()
{
	int i, j, k;
	int a, c;
	byte *vram = lcd.vbank[0];
	
	if (!anydirty) return;
	for (i = 0; i < 1024; i++)
	{
		if (i == 384) i = 512;
		if (i == 896) break;
		if (!patdirty[i]) continue;
		patdirty[i] = 0;
		for (j = 0; j < 8; j++)
		{
			a = ((i<<4) | (j<<1));
			for (k = 0; k < 8; k++)
			{
				c = vram[a] & (1<<k) ? 1 : 0;
				c |= vram[a+1] & (1<<k) ? 2 : 0;
				patpix[i+1024][j][k] = c;
			}
			for (k = 0; k < 8; k++)
				patpix[i][j][k] =
					patpix[i+1024][j][7-k];
		}
		for (j = 0; j < 8; j++)
		{
			for (k = 0; k < 8; k++)
			{
				patpix[i+2048][j][k] =
					patpix[i][7-j][k];
				patpix[i+3072][j][k] =
					patpix[i+1024][7-j][k];
			}
		}
	}
	anydirty = 0;
}
#endif /* ASM_UPDATEPATPIX */



void tilebuf()
{
	int i, cnt;
	int base;
	byte *tilemap, *attrmap;
	int *tilebuf;
	int *wrap;
	static int wraptable[64] =
	{
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,-32
	};

	base = ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5) + S;
	tilemap = lcd.vbank[0] + base;
	attrmap = lcd.vbank[1] + base;
	tilebuf = BG;
	wrap = wraptable + S;
	cnt = ((WX + 7) >> 3) + 1;

	if (hw.cgb)
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *tilemap
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*attrmap & 0x07) << 2);
				attrmap += *wrap + 1;
				tilemap += *(wrap++) + 1;
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*tilemap))
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*attrmap & 0x07) << 2);
				attrmap += *wrap + 1;
				tilemap += *(wrap++) + 1;
			}
	}
	else
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++);
				tilemap += *(wrap++);
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*(tilemap++)));
				tilemap += *(wrap++);
			}
	}

	if (WX >= 160) return;
	
	base = ((R_LCDC&0x40)?0x1C00:0x1800) + (WT<<5);
	tilemap = lcd.vbank[0] + base;
	attrmap = lcd.vbank[1] + base;
	tilebuf = WND;
	cnt = ((160 - WX) >> 3) + 1;

	if (hw.cgb)
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++)
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&7) << 2);
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*(tilemap++)))
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&7) << 2);
			}
	}
	else
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
				*(tilebuf++) = *(tilemap++);
		else
			for (i = cnt; i > 0; i--)
				*(tilebuf++) = (256 + ((n8)*(tilemap++)));
	}
}


void bg_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX <= 0) return;
	cnt = WX;
	tile = BG;
	dest = BUF;
	
	src = patpix[*(tile++)][V] + U;
	memcpy(dest, src, 8-U);
	dest += 8-U;
	cnt -= 8-U;
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][V];
		MEMCPY8(dest, src);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*tile][V];
	while (cnt--)
		*(dest++) = *(src++);
}

void wnd_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX >= 160) return;
	cnt = 160 - WX;
	tile = WND;
	dest = BUF + WX;
	
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][WV];
		MEMCPY8(dest, src);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*tile][WV];
	while (cnt--)
		*(dest++) = *(src++);
}

static void blendcpy(byte *dest, byte *src, byte b, int cnt)
{
	while (cnt--) *(dest++) = *(src++) | b;
}

static int priused(void *attr)
{
	un32 *a = attr;
	return (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);
}

void bg_scan_pri()
{
	int cnt, i;
	byte *src, *dest;

	if (WX <= 0) return;
	i = S;
	cnt = WX;
	dest = PRI;
	src = lcd.vbank[1] + ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5);

	if (!priused(src))
	{
		memset(dest, 0, cnt);
		return;
	}
	
	memset(dest, src[i++&31]&128, 8-U);
	dest += 8-U;
	cnt -= 8-U;
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		memset(dest, src[i++&31]&128, 8);
		dest += 8;
		cnt -= 8;
	}
	memset(dest, src[i&31]&128, cnt);
}

void wnd_scan_pri()
{
	int cnt, i;
	byte *src, *dest;

	if (WX >= 160) return;
	i = 0;
	cnt = 160 - WX;
	dest = PRI + WX;
	src = lcd.vbank[1] + ((R_LCDC&0x40)?0x1C00:0x1800) + (WT<<5);
	
	if (!priused(src))
	{
		memset(dest, 0, cnt);
		return;
	}
	
	while (cnt >= 8)
	{
		memset(dest, src[i++]&128, 8);
		dest += 8;
		cnt -= 8;
	}
	memset(dest, src[i]&128, cnt);
}

#ifndef ASM_BG_SCAN_COLOR
void bg_scan_color()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX <= 0) return;
	cnt = WX;
	tile = BG;
	dest = BUF;
	
	src = patpix[*(tile++)][V] + U;
	blendcpy(dest, src, *(tile++), 8-U);
	dest += 8-U;
	cnt -= 8-U;
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][V];
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*(tile++)][V];
	blendcpy(dest, src, *(tile++), cnt);
}
#endif

void wnd_scan_color()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX >= 160) return;
	cnt = 160 - WX;
	tile = WND;
	dest = BUF + WX;
	
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][WV];
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*(tile++)][WV];
	blendcpy(dest, src, *(tile++), cnt);
}

static void recolor(byte *buf, byte fill, int cnt)
{
	while (cnt--) *(buf++) |= fill;
}

void spr_count()
{
	int i;
	struct obj *o;
	
	NS = 0;
	if (!(R_LCDC & 0x02)) return;
	
	o = lcd.oam.obj;
	
	for (i = 40; i; i--, o++)
	{
		if (L >= o->y || L + 16 < o->y)
			continue;
		if (L + 8 >= o->y && !(R_LCDC & 0x04))
			continue;
		if (++NS == 10) break;
	}
}

void spr_enum()
{
	int i, j;
	struct obj *o;
	struct vissprite ts[10];
	int v, pat;
	int l, x;

	NS = 0;
	if (!(R_LCDC & 0x02)) return;

	o = lcd.oam.obj;
	
	for (i = 40; i; i--, o++)
	{
		if (L >= o->y || L + 16 < o->y)
			continue;
		if (L + 8 >= o->y && !(R_LCDC & 0x04))
			continue;
		VS[NS].x = (int)o->x - 8;
		v = L - (int)o->y + 16;
		if (hw.cgb)
		{
			pat = o->pat | (((int)o->flags & 0x60) << 5)
				| (((int)o->flags & 0x08) << 6);
			VS[NS].pal = 32 + ((o->flags & 0x07) << 2);
		}
		else
		{
			pat = o->pat | (((int)o->flags & 0x60) << 5);
			VS[NS].pal = 32 + ((o->flags & 0x10) >> 2);
		}
		VS[NS].pri = (o->flags & 0x80) >> 7;
		if ((R_LCDC & 0x04))
		{
			pat &= ~1;
			if (v >= 8)
			{
				v -= 8;
				pat++;
			}
			if (o->flags & 0x40) pat ^= 1;
		}
		VS[NS].buf = patpix[pat][v];
		if (++NS == 10) break;
	}
	if (!sprsort || hw.cgb) return;
	/* not quite optimal but it finally works! */
	for (i = 0; i < NS; i++)
	{
		l = 0;
		x = VS[0].x;
		for (j = 1; j < NS; j++)
		{
			if (VS[j].x < x)
			{
				l = j;
				x = VS[j].x;
			}
		}
		ts[i] = VS[l];
		VS[l].x = 160;
	}
	memcpy(VS, ts, sizeof VS);
}

void spr_scan()
{
	int i, x;
	byte pal, b, ns = NS;
	byte *src, *dest, *bg, *pri;
	struct vissprite *vs;
	static byte bgdup[256];

	if (!ns) return;

	memcpy(bgdup, BUF, 256);
	vs = &VS[ns-1];
	
	for (; ns; ns--, vs--)
	{
		x = vs->x;
		if (x >= 160) continue;
		if (x <= -8) continue;
		if (x < 0)
		{
			src = vs->buf - x;
			dest = BUF;
			i = 8 + x;
		}
		else
		{
			src = vs->buf;
			dest = BUF + x;
			if (x > 152) i = 160 - x;
			else i = 8;
		}
		pal = vs->pal;
		if (vs->pri)
		{
			bg = bgdup + (dest - BUF);
			while (i--)
			{
				b = src[i];
				if (b && !(bg[i]&3)) dest[i] = pal|b;
			}
		}
		else if (hw.cgb)
		{
			bg = bgdup + (dest - BUF);
			pri = PRI + (dest - BUF);
			while (i--)
			{
				b = src[i];
				if (b && (!pri[i] || !(bg[i]&3)))
					dest[i] = pal|b;
			}
		}
		else while (i--) if (src[i]) dest[i] = pal|src[i];
		/* else while (i--) if (src[i]) dest[i] = 31 + ns; */
	}
	if (sprdebug) for (i = 0; i < NS; i++) BUF[i<<1] = 36;
}



void lcd_begin()
{
	if (fb.indexed)
	{
		if (rgb332) pal_set332();
		else pal_expire();
	}
	while (scale * 160 > fb.w || scale * 144 > fb.h) scale--;
	vdest = fb.ptr + ((fb.w*fb.pelsize)>>1)
		- (80*fb.pelsize) * scale
		+ ((fb.h>>1) - 72*scale) * fb.pitch;
	WY = R_WY;
}

void lcd_refreshline()
{
	int i;
	byte scalebuf[160*4*4], *dest;
	
	if (!fb.enabled) return;
	
	if (!(R_LCDC & 0x80))
		return; /* should not happen... */
	
	updatepatpix();

	L = R_LY;
	X = R_SCX;
	Y = (R_SCY + L) & 0xff;
	S = X >> 3;
	T = Y >> 3;
	U = X & 7;
	V = Y & 7;
	
	WX = R_WX - 7;
	if (WY>L || WY<0 || WY>143 || WX<-7 || WX>159 || !(R_LCDC&0x20))
		WX = 160;
	WT = (L - WY) >> 3;
	WV = (L - WY) & 7;

	spr_enum();

	tilebuf();
	if (hw.cgb)
	{
		bg_scan_color();
		wnd_scan_color();
		if (NS)
		{
			bg_scan_pri();
			wnd_scan_pri();
		}
	}
	else
	{
		bg_scan();
		wnd_scan();
		recolor(BUF+WX, 0x04, 160-WX);
	}
	spr_scan();

	if (fb.dirty) memset(fb.ptr, 0, fb.pitch * fb.h);
	fb.dirty = 0;
	if (density > scale) density = scale;
	if (scale == 1) density = 1;

	dest = (density != 1) ? scalebuf : vdest;
	
	switch (scale)
	{
	case 0:
	case 1:
		switch (fb.pelsize)
		{
		case 1:
			refresh_1(dest, BUF, PAL1, 160);
			break;
		case 2:
			refresh_2((un16 *)dest, BUF, PAL2, 160);
			break;
		case 3:
			refresh_3(dest, BUF, PAL4, 160);
			break;
		case 4:
			refresh_4((un32 *)dest, BUF, PAL4, 160);
			break;
		}
		break;
	case 2:
		switch (fb.pelsize)
		{
		case 1:
			refresh_2((un16 *)dest, BUF, PAL2, 160);
			break;
		case 2:
			refresh_4((un32 *)dest, BUF, PAL4, 160);
			break;
		case 3:
			refresh_3_2x(dest, BUF, PAL4, 160);
			break;
		case 4:
			refresh_4_2x((un32 *)dest, BUF, PAL4, 160);
			break;
		}
		break;
	case 3:
		switch (fb.pelsize)
		{
		case 1:
			refresh_3(dest, BUF, PAL4, 160);
			break;
		case 2:
			refresh_2_3x((un16 *)dest, BUF, PAL2, 160);
			break;
		case 3:
			refresh_3_3x(dest, BUF, PAL4, 160);
			break;
		case 4:
			refresh_4_3x((un32 *)dest, BUF, PAL4, 160);
			break;
		}
		break;
	case 4:
		switch (fb.pelsize)
		{
		case 1:
			refresh_4((un32 *)dest, BUF, PAL4, 160);
			break;
		case 2:
			refresh_4_2x((un32 *)dest, BUF, PAL4, 160);
			break;
		case 3:
			refresh_3_4x(dest, BUF, PAL4, 160);
			break;
		case 4:
			refresh_4_4x((un32 *)dest, BUF, PAL4, 160);
			break;
		}
		break;
	default:
		break;
	}

	if (density != 1)
	{
		for (i = 0; i < scale; i++)
		{
			if ((i < density) || ((density <= 0) && !(i&1)))
				memcpy(vdest, scalebuf, 160 * fb.pelsize * scale);
			vdest += fb.pitch;
		}
	}
	else vdest += fb.pitch * scale;
}







static void updatepalette(int i)
{
	int c, r, g, b, y, u, v, rr, gg;

	c = (lcd.pal[i<<1] | ((int)lcd.pal[(i<<1)|1] << 8)) & 0x7FFF;
	r = (c & 0x001F) << 3;
	g = (c & 0x03E0) >> 2;
	b = (c & 0x7C00) >> 7;
	r |= (r >> 5);
	g |= (g >> 5);
	b |= (b >> 5);

	if (usefilter && (filterdmg || hw.cgb))
	{
		rr = ((r * filter[0][0] + g * filter[0][1] + b * filter[0][2]) >> 8) + filter[0][3];
		gg = ((r * filter[1][0] + g * filter[1][1] + b * filter[1][2]) >> 8) + filter[1][3];
		b = ((r * filter[2][0] + g * filter[2][1] + b * filter[2][2]) >> 8) + filter[2][3];
		r = rr;
		g = gg;
	}
	
	if (fb.yuv)
	{
		y = (((r *  263) + (g * 516) + (b * 100)) >> 10) + 16;
		u = (((r *  450) - (g * 377) - (b *  73)) >> 10) + 128;
		v = (((r * -152) - (g * 298) + (b * 450)) >> 10) + 128;
		if (y < 0) y = 0; if (y > 255) y = 255;
		if (u < 0) u = 0; if (u > 255) u = 255;
		if (v < 0) v = 0; if (v > 255) v = 255;
		PAL4[i] = (y<<fb.cc[0].l) | (y<<fb.cc[3].l)
			| (u<<fb.cc[1].l) | (v<<fb.cc[2].l);
		return;
	}
	
	if (fb.indexed)
	{
		pal_release(PAL1[i]);
		c = pal_getcolor(c, r, g, b);
		PAL1[i] = c;
		PAL2[i] = (c<<8) | c;
		PAL4[i] = (c<<24) | (c<<16) | (c<<8) | c;
		return;
	}

	r = (r >> fb.cc[0].r) << fb.cc[0].l;
	g = (g >> fb.cc[1].r) << fb.cc[1].l;
	b = (b >> fb.cc[2].r) << fb.cc[2].l;
	c = r|g|b;
	
	switch (fb.pelsize)
	{
	case 1:
		PAL1[i] = c;
		PAL2[i] = (c<<8) | c;
		PAL4[i] = (c<<24) | (c<<16) | (c<<8) | c;
		break;
	case 2:
		PAL2[i] = c;
		PAL4[i] = (c<<16) | c;
		break;
	case 3:
	case 4:
		PAL4[i] = c;
		break;
	}
}

void pal_write(int i, byte b)
{
	if (lcd.pal[i] == b) return;
	lcd.pal[i] = b;
	updatepalette(i>>1);
}

void pal_write_dmg(int i, int mapnum, byte d)
{
	int j;
	int *cmap = dmg_pal[paletteindex][mapnum];
	int c, r, g, b;

	if (hw.cgb) return;

	/* if (mapnum >= 2) d = 0xe4; */
	for (j = 0; j < 8; j += 2)
	{
		c = cmap[(d >> j) & 3];
		r = (c & 0xf8) >> 3;
		g = (c & 0xf800) >> 6;
		b = (c & 0xf80000) >> 9;
		c = r|g|b;
		/* FIXME - handle directly without faking cgb */
		pal_write(i+j, c & 0xff);
		pal_write(i+j+1, c >> 8);
	}
}

void vram_write(int a, byte b)
{
	lcd.vbank[R_VBK&1][a] = b;
	if (a >= 0x1800) return;
	patdirty[((R_VBK&1)<<9)+(a>>4)] = 1;
	anydirty = 1;
}

void vram_dirty()
{
	anydirty = 1;
	memset(patdirty, 1, sizeof patdirty);
}

void pal_dirty()
{
	int i;
	if (!hw.cgb)
	{
		pal_write_dmg(0, 0, R_BGP);
		pal_write_dmg(8, 1, R_BGP);
		pal_write_dmg(64, 2, R_OBP0);
		pal_write_dmg(72, 3, R_OBP1);
	}
	for (i = 0; i < 64; i++)
		updatepalette(i);
}

void lcd_reset()
{
	memset(&lcd, 0, sizeof lcd);
	lcd_begin();
	vram_dirty();
	pal_dirty();
}
