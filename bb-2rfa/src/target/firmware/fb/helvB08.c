#include <fb/font.h>
static const uint8_t font_helvB08_data[] = {
/* --- new character space (32) starting at offset 0x0000 --- */
	/*0000:*/	2, 1, 1, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0005:*/	0x00,  /* ........ */
/* --- new character exclam (33) starting at offset 0x0006 --- */
	/*0006:*/	3, 2, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*000b:*/	0xc0,  /* ##...... */
	/*000c:*/	0xc0,  /* ##...... */
	/*000d:*/	0xc0,  /* ##...... */
	/*000e:*/	0x80,  /* #....... */
	/*000f:*/	0x00,  /* ........ */
	/*0010:*/	0x80,  /* #....... */
	/*0011:*/	0x80,  /* #....... */
/* --- new character quotedbl (34) starting at offset 0x0012 --- */
	/*0012:*/	4, 3, 2, 0, 4, /* width and bbox (w,h,x,y) */ 
	/*0017:*/	0xa0,  /* #.#..... */
	/*0018:*/	0xa0,  /* #.#..... */
/* --- new character numbersign (35) starting at offset 0x0019 --- */
	/*0019:*/	5, 5, 6, -1, 0, /* width and bbox (w,h,x,y) */ 
	/*001e:*/	0x50,  /* .#.#.... */
	/*001f:*/	0xf8,  /* #####... */
	/*0020:*/	0x50,  /* .#.#.... */
	/*0021:*/	0xf8,  /* #####... */
	/*0022:*/	0xa0,  /* #.#..... */
	/*0023:*/	0xa0,  /* #.#..... */
/* --- new character dollar (36) starting at offset 0x0024 --- */
	/*0024:*/	5, 4, 8, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*0029:*/	0x20,  /* ..#..... */
	/*002a:*/	0x70,  /* .###.... */
	/*002b:*/	0xc0,  /* ##...... */
	/*002c:*/	0xe0,  /* ###..... */
	/*002d:*/	0x70,  /* .###.... */
	/*002e:*/	0x30,  /* ..##.... */
	/*002f:*/	0xe0,  /* ###..... */
	/*0030:*/	0x40,  /* .#...... */
/* --- new character percent (37) starting at offset 0x0031 --- */
	/*0031:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0036:*/	0x68,  /* .##.#... */
	/*0037:*/	0xb0,  /* #.##.... */
	/*0038:*/	0xe0,  /* ###..... */
	/*0039:*/	0x38,  /* ..###... */
	/*003a:*/	0x68,  /* .##.#... */
	/*003b:*/	0xb0,  /* #.##.... */
/* --- new character ampersand (38) starting at offset 0x003c --- */
	/*003c:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0041:*/	0x70,  /* .###.... */
	/*0042:*/	0x50,  /* .#.#.... */
	/*0043:*/	0x60,  /* .##..... */
	/*0044:*/	0xf8,  /* #####... */
	/*0045:*/	0xd0,  /* ##.#.... */
	/*0046:*/	0x68,  /* .##.#... */
/* --- new character quotesingle (39) starting at offset 0x0047 --- */
	/*0047:*/	3, 1, 3, 1, 5, /* width and bbox (w,h,x,y) */ 
	/*004c:*/	0x80,  /* #....... */
	/*004d:*/	0x80,  /* #....... */
	/*004e:*/	0x80,  /* #....... */
/* --- new character parenleft (40) starting at offset 0x004f --- */
	/*004f:*/	3, 2, 8, 0, -2, /* width and bbox (w,h,x,y) */ 
	/*0054:*/	0x40,  /* .#...... */
	/*0055:*/	0x40,  /* .#...... */
	/*0056:*/	0x80,  /* #....... */
	/*0057:*/	0x80,  /* #....... */
	/*0058:*/	0x80,  /* #....... */
	/*0059:*/	0x80,  /* #....... */
	/*005a:*/	0x40,  /* .#...... */
	/*005b:*/	0x40,  /* .#...... */
/* --- new character parenright (41) starting at offset 0x005c --- */
	/*005c:*/	3, 2, 8, 0, -2, /* width and bbox (w,h,x,y) */ 
	/*0061:*/	0x80,  /* #....... */
	/*0062:*/	0x80,  /* #....... */
	/*0063:*/	0x40,  /* .#...... */
	/*0064:*/	0x40,  /* .#...... */
	/*0065:*/	0x40,  /* .#...... */
	/*0066:*/	0x40,  /* .#...... */
	/*0067:*/	0x80,  /* #....... */
	/*0068:*/	0x80,  /* #....... */
/* --- new character asterisk (42) starting at offset 0x0069 --- */
	/*0069:*/	3, 3, 3, 0, 3, /* width and bbox (w,h,x,y) */ 
	/*006e:*/	0x40,  /* .#...... */
	/*006f:*/	0xe0,  /* ###..... */
	/*0070:*/	0x40,  /* .#...... */
/* --- new character plus (43) starting at offset 0x0071 --- */
	/*0071:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0076:*/	0x20,  /* ..#..... */
	/*0077:*/	0x20,  /* ..#..... */
	/*0078:*/	0xf0,  /* ####.... */
	/*0079:*/	0x20,  /* ..#..... */
	/*007a:*/	0x20,  /* ..#..... */
/* --- new character comma (44) starting at offset 0x007b --- */
	/*007b:*/	2, 2, 3, -1, -1, /* width and bbox (w,h,x,y) */ 
	/*0080:*/	0x40,  /* .#...... */
	/*0081:*/	0x40,  /* .#...... */
	/*0082:*/	0x80,  /* #....... */
/* --- new character hyphen (45) starting at offset 0x0083 --- */
	/*0083:*/	4, 3, 1, 0, 2, /* width and bbox (w,h,x,y) */ 
	/*0088:*/	0xe0,  /* ###..... */
/* --- new character period (46) starting at offset 0x0089 --- */
	/*0089:*/	2, 1, 2, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*008e:*/	0x80,  /* #....... */
	/*008f:*/	0x80,  /* #....... */
/* --- new character slash (47) starting at offset 0x0090 --- */
	/*0090:*/	3, 3, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0095:*/	0x20,  /* ..#..... */
	/*0096:*/	0x20,  /* ..#..... */
	/*0097:*/	0x40,  /* .#...... */
	/*0098:*/	0x40,  /* .#...... */
	/*0099:*/	0x80,  /* #....... */
	/*009a:*/	0x80,  /* #....... */
/* --- new character zero (48) starting at offset 0x009b --- */
	/*009b:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00a0:*/	0x60,  /* .##..... */
	/*00a1:*/	0xd0,  /* ##.#.... */
	/*00a2:*/	0xd0,  /* ##.#.... */
	/*00a3:*/	0xd0,  /* ##.#.... */
	/*00a4:*/	0xd0,  /* ##.#.... */
	/*00a5:*/	0x60,  /* .##..... */
/* --- new character one (49) starting at offset 0x00a6 --- */
	/*00a6:*/	5, 3, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00ab:*/	0x20,  /* ..#..... */
	/*00ac:*/	0xe0,  /* ###..... */
	/*00ad:*/	0x60,  /* .##..... */
	/*00ae:*/	0x60,  /* .##..... */
	/*00af:*/	0x60,  /* .##..... */
	/*00b0:*/	0x60,  /* .##..... */
/* --- new character two (50) starting at offset 0x00b1 --- */
	/*00b1:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00b6:*/	0x60,  /* .##..... */
	/*00b7:*/	0xb0,  /* #.##.... */
	/*00b8:*/	0x30,  /* ..##.... */
	/*00b9:*/	0x60,  /* .##..... */
	/*00ba:*/	0xc0,  /* ##...... */
	/*00bb:*/	0xf0,  /* ####.... */
/* --- new character three (51) starting at offset 0x00bc --- */
	/*00bc:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00c1:*/	0x60,  /* .##..... */
	/*00c2:*/	0xb0,  /* #.##.... */
	/*00c3:*/	0x60,  /* .##..... */
	/*00c4:*/	0x30,  /* ..##.... */
	/*00c5:*/	0xb0,  /* #.##.... */
	/*00c6:*/	0x60,  /* .##..... */
/* --- new character four (52) starting at offset 0x00c7 --- */
	/*00c7:*/	5, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00cc:*/	0x30,  /* ..##.... */
	/*00cd:*/	0x50,  /* .#.#.... */
	/*00ce:*/	0xd0,  /* ##.#.... */
	/*00cf:*/	0xf8,  /* #####... */
	/*00d0:*/	0x30,  /* ..##.... */
	/*00d1:*/	0x30,  /* ..##.... */
/* --- new character five (53) starting at offset 0x00d2 --- */
	/*00d2:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00d7:*/	0x70,  /* .###.... */
	/*00d8:*/	0xc0,  /* ##...... */
	/*00d9:*/	0xe0,  /* ###..... */
	/*00da:*/	0x30,  /* ..##.... */
	/*00db:*/	0xb0,  /* #.##.... */
	/*00dc:*/	0x60,  /* .##..... */
/* --- new character six (54) starting at offset 0x00dd --- */
	/*00dd:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00e2:*/	0x70,  /* .###.... */
	/*00e3:*/	0xc0,  /* ##...... */
	/*00e4:*/	0xe0,  /* ###..... */
	/*00e5:*/	0xd0,  /* ##.#.... */
	/*00e6:*/	0xd0,  /* ##.#.... */
	/*00e7:*/	0x60,  /* .##..... */
/* --- new character seven (55) starting at offset 0x00e8 --- */
	/*00e8:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00ed:*/	0xf0,  /* ####.... */
	/*00ee:*/	0x30,  /* ..##.... */
	/*00ef:*/	0x30,  /* ..##.... */
	/*00f0:*/	0x60,  /* .##..... */
	/*00f1:*/	0x40,  /* .#...... */
	/*00f2:*/	0xc0,  /* ##...... */
/* --- new character eight (56) starting at offset 0x00f3 --- */
	/*00f3:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*00f8:*/	0x60,  /* .##..... */
	/*00f9:*/	0xd0,  /* ##.#.... */
	/*00fa:*/	0x60,  /* .##..... */
	/*00fb:*/	0xd0,  /* ##.#.... */
	/*00fc:*/	0xd0,  /* ##.#.... */
	/*00fd:*/	0x60,  /* .##..... */
/* --- new character nine (57) starting at offset 0x00fe --- */
	/*00fe:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0103:*/	0x60,  /* .##..... */
	/*0104:*/	0xb0,  /* #.##.... */
	/*0105:*/	0xb0,  /* #.##.... */
	/*0106:*/	0x70,  /* .###.... */
	/*0107:*/	0x30,  /* ..##.... */
	/*0108:*/	0xe0,  /* ###..... */
/* --- new character colon (58) starting at offset 0x0109 --- */
	/*0109:*/	2, 1, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*010e:*/	0x80,  /* #....... */
	/*010f:*/	0x80,  /* #....... */
	/*0110:*/	0x00,  /* ........ */
	/*0111:*/	0x80,  /* #....... */
	/*0112:*/	0x80,  /* #....... */
/* --- new character semicolon (59) starting at offset 0x0113 --- */
	/*0113:*/	2, 2, 6, -1, -1, /* width and bbox (w,h,x,y) */ 
	/*0118:*/	0x40,  /* .#...... */
	/*0119:*/	0x40,  /* .#...... */
	/*011a:*/	0x00,  /* ........ */
	/*011b:*/	0x40,  /* .#...... */
	/*011c:*/	0x40,  /* .#...... */
	/*011d:*/	0x80,  /* #....... */
/* --- new character less (60) starting at offset 0x011e --- */
	/*011e:*/	4, 3, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0123:*/	0x20,  /* ..#..... */
	/*0124:*/	0x40,  /* .#...... */
	/*0125:*/	0x80,  /* #....... */
	/*0126:*/	0x40,  /* .#...... */
	/*0127:*/	0x20,  /* ..#..... */
/* --- new character equal (61) starting at offset 0x0128 --- */
	/*0128:*/	5, 4, 3, 0, 1, /* width and bbox (w,h,x,y) */ 
	/*012d:*/	0xf0,  /* ####.... */
	/*012e:*/	0x00,  /* ........ */
	/*012f:*/	0xf0,  /* ####.... */
/* --- new character greater (62) starting at offset 0x0130 --- */
	/*0130:*/	4, 3, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0135:*/	0x80,  /* #....... */
	/*0136:*/	0x40,  /* .#...... */
	/*0137:*/	0x20,  /* ..#..... */
	/*0138:*/	0x40,  /* .#...... */
	/*0139:*/	0x80,  /* #....... */
/* --- new character question (63) starting at offset 0x013a --- */
	/*013a:*/	5, 4, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*013f:*/	0xe0,  /* ###..... */
	/*0140:*/	0x30,  /* ..##.... */
	/*0141:*/	0x60,  /* .##..... */
	/*0142:*/	0x40,  /* .#...... */
	/*0143:*/	0x00,  /* ........ */
	/*0144:*/	0x40,  /* .#...... */
	/*0145:*/	0x40,  /* .#...... */
/* --- new character at (64) starting at offset 0x0146 --- */
	/*0146:*/	9, 8, 7, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*014b:*/	0x7e,  /* .######. */
	/*014c:*/	0xc3,  /* ##....## */
	/*014d:*/	0x99,  /* #..##..# */
	/*014e:*/	0xa9,  /* #.#.#..# */
	/*014f:*/	0x99,  /* #..##..# */
	/*0150:*/	0xce,  /* ##..###. */
	/*0151:*/	0x60,  /* .##..... */
/* --- new character A (65) starting at offset 0x0152 --- */
	/*0152:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0157:*/	0x70,  /* .###.... */
	/*0158:*/	0xd8,  /* ##.##... */
	/*0159:*/	0xd8,  /* ##.##... */
	/*015a:*/	0xf8,  /* #####... */
	/*015b:*/	0xd8,  /* ##.##... */
	/*015c:*/	0xd8,  /* ##.##... */
/* --- new character B (66) starting at offset 0x015d --- */
	/*015d:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0162:*/	0xf0,  /* ####.... */
	/*0163:*/	0xd8,  /* ##.##... */
	/*0164:*/	0xf0,  /* ####.... */
	/*0165:*/	0xd8,  /* ##.##... */
	/*0166:*/	0xd8,  /* ##.##... */
	/*0167:*/	0xf0,  /* ####.... */
/* --- new character C (67) starting at offset 0x0168 --- */
	/*0168:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*016d:*/	0x78,  /* .####... */
	/*016e:*/	0xc8,  /* ##..#... */
	/*016f:*/	0xc0,  /* ##...... */
	/*0170:*/	0xc0,  /* ##...... */
	/*0171:*/	0xc8,  /* ##..#... */
	/*0172:*/	0x78,  /* .####... */
/* --- new character D (68) starting at offset 0x0173 --- */
	/*0173:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0178:*/	0xf0,  /* ####.... */
	/*0179:*/	0xd8,  /* ##.##... */
	/*017a:*/	0xd8,  /* ##.##... */
	/*017b:*/	0xd8,  /* ##.##... */
	/*017c:*/	0xd8,  /* ##.##... */
	/*017d:*/	0xf0,  /* ####.... */
/* --- new character E (69) starting at offset 0x017e --- */
	/*017e:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0183:*/	0xf0,  /* ####.... */
	/*0184:*/	0xc0,  /* ##...... */
	/*0185:*/	0xf0,  /* ####.... */
	/*0186:*/	0xc0,  /* ##...... */
	/*0187:*/	0xc0,  /* ##...... */
	/*0188:*/	0xf0,  /* ####.... */
/* --- new character F (70) starting at offset 0x0189 --- */
	/*0189:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*018e:*/	0xf0,  /* ####.... */
	/*018f:*/	0xc0,  /* ##...... */
	/*0190:*/	0xf0,  /* ####.... */
	/*0191:*/	0xc0,  /* ##...... */
	/*0192:*/	0xc0,  /* ##...... */
	/*0193:*/	0xc0,  /* ##...... */
/* --- new character G (71) starting at offset 0x0194 --- */
	/*0194:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0199:*/	0x78,  /* .####... */
	/*019a:*/	0xc8,  /* ##..#... */
	/*019b:*/	0xc0,  /* ##...... */
	/*019c:*/	0xd8,  /* ##.##... */
	/*019d:*/	0xc8,  /* ##..#... */
	/*019e:*/	0x78,  /* .####... */
/* --- new character H (72) starting at offset 0x019f --- */
	/*019f:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01a4:*/	0xd8,  /* ##.##... */
	/*01a5:*/	0xd8,  /* ##.##... */
	/*01a6:*/	0xf8,  /* #####... */
	/*01a7:*/	0xd8,  /* ##.##... */
	/*01a8:*/	0xd8,  /* ##.##... */
	/*01a9:*/	0xd8,  /* ##.##... */
/* --- new character I (73) starting at offset 0x01aa --- */
	/*01aa:*/	2, 1, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01af:*/	0x80,  /* #....... */
	/*01b0:*/	0x80,  /* #....... */
	/*01b1:*/	0x80,  /* #....... */
	/*01b2:*/	0x80,  /* #....... */
	/*01b3:*/	0x80,  /* #....... */
	/*01b4:*/	0x80,  /* #....... */
/* --- new character J (74) starting at offset 0x01b5 --- */
	/*01b5:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01ba:*/	0x30,  /* ..##.... */
	/*01bb:*/	0x30,  /* ..##.... */
	/*01bc:*/	0x30,  /* ..##.... */
	/*01bd:*/	0x30,  /* ..##.... */
	/*01be:*/	0xb0,  /* #.##.... */
	/*01bf:*/	0x60,  /* .##..... */
/* --- new character K (75) starting at offset 0x01c0 --- */
	/*01c0:*/	6, 6, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01c5:*/	0xd8,  /* ##.##... */
	/*01c6:*/	0xd0,  /* ##.#.... */
	/*01c7:*/	0xe0,  /* ###..... */
	/*01c8:*/	0xf0,  /* ####.... */
	/*01c9:*/	0xd8,  /* ##.##... */
	/*01ca:*/	0xcc,  /* ##..##.. */
/* --- new character L (76) starting at offset 0x01cb --- */
	/*01cb:*/	5, 4, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01d0:*/	0xc0,  /* ##...... */
	/*01d1:*/	0xc0,  /* ##...... */
	/*01d2:*/	0xc0,  /* ##...... */
	/*01d3:*/	0xc0,  /* ##...... */
	/*01d4:*/	0xc0,  /* ##...... */
	/*01d5:*/	0xf0,  /* ####.... */
/* --- new character M (77) starting at offset 0x01d6 --- */
	/*01d6:*/	8, 7, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01db:*/	0xc6,  /* ##...##. */
	/*01dc:*/	0xc6,  /* ##...##. */
	/*01dd:*/	0xee,  /* ###.###. */
	/*01de:*/	0xfe,  /* #######. */
	/*01df:*/	0xd6,  /* ##.#.##. */
	/*01e0:*/	0xd6,  /* ##.#.##. */
/* --- new character N (78) starting at offset 0x01e1 --- */
	/*01e1:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01e6:*/	0xc8,  /* ##..#... */
	/*01e7:*/	0xc8,  /* ##..#... */
	/*01e8:*/	0xe8,  /* ###.#... */
	/*01e9:*/	0xf8,  /* #####... */
	/*01ea:*/	0xd8,  /* ##.##... */
	/*01eb:*/	0xc8,  /* ##..#... */
/* --- new character O (79) starting at offset 0x01ec --- */
	/*01ec:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01f1:*/	0x70,  /* .###.... */
	/*01f2:*/	0xd8,  /* ##.##... */
	/*01f3:*/	0xc8,  /* ##..#... */
	/*01f4:*/	0xc8,  /* ##..#... */
	/*01f5:*/	0xd8,  /* ##.##... */
	/*01f6:*/	0x70,  /* .###.... */
/* --- new character P (80) starting at offset 0x01f7 --- */
	/*01f7:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*01fc:*/	0xf0,  /* ####.... */
	/*01fd:*/	0xd8,  /* ##.##... */
	/*01fe:*/	0xd8,  /* ##.##... */
	/*01ff:*/	0xf0,  /* ####.... */
	/*0200:*/	0xc0,  /* ##...... */
	/*0201:*/	0xc0,  /* ##...... */
/* --- new character Q (81) starting at offset 0x0202 --- */
	/*0202:*/	6, 6, 7, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*0207:*/	0x70,  /* .###.... */
	/*0208:*/	0xd8,  /* ##.##... */
	/*0209:*/	0xc8,  /* ##..#... */
	/*020a:*/	0xc8,  /* ##..#... */
	/*020b:*/	0xd8,  /* ##.##... */
	/*020c:*/	0x78,  /* .####... */
	/*020d:*/	0x04,  /* .....#.. */
/* --- new character R (82) starting at offset 0x020e --- */
	/*020e:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0213:*/	0xf0,  /* ####.... */
	/*0214:*/	0xd8,  /* ##.##... */
	/*0215:*/	0xd8,  /* ##.##... */
	/*0216:*/	0xf0,  /* ####.... */
	/*0217:*/	0xd8,  /* ##.##... */
	/*0218:*/	0xd8,  /* ##.##... */
/* --- new character S (83) starting at offset 0x0219 --- */
	/*0219:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*021e:*/	0x78,  /* .####... */
	/*021f:*/	0xc0,  /* ##...... */
	/*0220:*/	0xf0,  /* ####.... */
	/*0221:*/	0x38,  /* ..###... */
	/*0222:*/	0xd8,  /* ##.##... */
	/*0223:*/	0x70,  /* .###.... */
/* --- new character T (84) starting at offset 0x0224 --- */
	/*0224:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0229:*/	0xf8,  /* #####... */
	/*022a:*/	0x60,  /* .##..... */
	/*022b:*/	0x60,  /* .##..... */
	/*022c:*/	0x60,  /* .##..... */
	/*022d:*/	0x60,  /* .##..... */
	/*022e:*/	0x60,  /* .##..... */
/* --- new character U (85) starting at offset 0x022f --- */
	/*022f:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0234:*/	0xd8,  /* ##.##... */
	/*0235:*/	0xd8,  /* ##.##... */
	/*0236:*/	0xd8,  /* ##.##... */
	/*0237:*/	0xd8,  /* ##.##... */
	/*0238:*/	0xd8,  /* ##.##... */
	/*0239:*/	0x70,  /* .###.... */
/* --- new character V (86) starting at offset 0x023a --- */
	/*023a:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*023f:*/	0xe8,  /* ###.#... */
	/*0240:*/	0x68,  /* .##.#... */
	/*0241:*/	0x68,  /* .##.#... */
	/*0242:*/	0x68,  /* .##.#... */
	/*0243:*/	0x70,  /* .###.... */
	/*0244:*/	0x20,  /* ..#..... */
/* --- new character W (87) starting at offset 0x0245 --- */
	/*0245:*/	9, 8, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*024a:*/	0xdb,  /* ##.##.## */
	/*024b:*/	0xdb,  /* ##.##.## */
	/*024c:*/	0xda,  /* ##.##.#. */
	/*024d:*/	0xda,  /* ##.##.#. */
	/*024e:*/	0x6c,  /* .##.##.. */
	/*024f:*/	0x6c,  /* .##.##.. */
/* --- new character X (88) starting at offset 0x0250 --- */
	/*0250:*/	6, 5, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0255:*/	0xd8,  /* ##.##... */
	/*0256:*/	0xd8,  /* ##.##... */
	/*0257:*/	0x70,  /* .###.... */
	/*0258:*/	0x70,  /* .###.... */
	/*0259:*/	0xd8,  /* ##.##... */
	/*025a:*/	0xd8,  /* ##.##... */
/* --- new character Y (89) starting at offset 0x025b --- */
	/*025b:*/	7, 6, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0260:*/	0xec,  /* ###.##.. */
	/*0261:*/	0x68,  /* .##.#... */
	/*0262:*/	0x68,  /* .##.#... */
	/*0263:*/	0x78,  /* .####... */
	/*0264:*/	0x30,  /* ..##.... */
	/*0265:*/	0x30,  /* ..##.... */
/* --- new character Z (90) starting at offset 0x0266 --- */
	/*0266:*/	6, 6, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*026b:*/	0xfc,  /* ######.. */
	/*026c:*/	0x38,  /* ..###... */
	/*026d:*/	0x30,  /* ..##.... */
	/*026e:*/	0x60,  /* .##..... */
	/*026f:*/	0xe0,  /* ###..... */
	/*0270:*/	0xf8,  /* #####... */
/* --- new character bracketleft (91) starting at offset 0x0271 --- */
	/*0271:*/	3, 2, 8, 0, -2, /* width and bbox (w,h,x,y) */ 
	/*0276:*/	0xc0,  /* ##...... */
	/*0277:*/	0x80,  /* #....... */
	/*0278:*/	0x80,  /* #....... */
	/*0279:*/	0x80,  /* #....... */
	/*027a:*/	0x80,  /* #....... */
	/*027b:*/	0x80,  /* #....... */
	/*027c:*/	0x80,  /* #....... */
	/*027d:*/	0xc0,  /* ##...... */
/* --- new character backslash (92) starting at offset 0x027e --- */
	/*027e:*/	3, 3, 6, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0283:*/	0x80,  /* #....... */
	/*0284:*/	0x80,  /* #....... */
	/*0285:*/	0x40,  /* .#...... */
	/*0286:*/	0x40,  /* .#...... */
	/*0287:*/	0x20,  /* ..#..... */
	/*0288:*/	0x20,  /* ..#..... */
/* --- new character bracketright (93) starting at offset 0x0289 --- */
	/*0289:*/	3, 2, 8, 0, -2, /* width and bbox (w,h,x,y) */ 
	/*028e:*/	0xc0,  /* ##...... */
	/*028f:*/	0x40,  /* .#...... */
	/*0290:*/	0x40,  /* .#...... */
	/*0291:*/	0x40,  /* .#...... */
	/*0292:*/	0x40,  /* .#...... */
	/*0293:*/	0x40,  /* .#...... */
	/*0294:*/	0x40,  /* .#...... */
	/*0295:*/	0xc0,  /* ##...... */
/* --- new character asciicircum (94) starting at offset 0x0296 --- */
	/*0296:*/	4, 4, 3, 0, 3, /* width and bbox (w,h,x,y) */ 
	/*029b:*/	0x60,  /* .##..... */
	/*029c:*/	0x60,  /* .##..... */
	/*029d:*/	0x90,  /* #..#.... */
/* --- new character underscore (95) starting at offset 0x029e --- */
	/*029e:*/	5, 5, 1, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*02a3:*/	0xf8,  /* #####... */
/* --- new character grave (96) starting at offset 0x02a4 --- */
	/*02a4:*/	3, 2, 2, 0, 6, /* width and bbox (w,h,x,y) */ 
	/*02a9:*/	0x80,  /* #....... */
	/*02aa:*/	0x40,  /* .#...... */
/* --- new character a (97) starting at offset 0x02ab --- */
	/*02ab:*/	5, 5, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*02b0:*/	0xe0,  /* ###..... */
	/*02b1:*/	0x30,  /* ..##.... */
	/*02b2:*/	0xf0,  /* ####.... */
	/*02b3:*/	0xb0,  /* #.##.... */
	/*02b4:*/	0xd8,  /* ##.##... */
/* --- new character b (98) starting at offset 0x02b5 --- */
	/*02b5:*/	5, 4, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*02ba:*/	0xc0,  /* ##...... */
	/*02bb:*/	0xc0,  /* ##...... */
	/*02bc:*/	0xe0,  /* ###..... */
	/*02bd:*/	0xd0,  /* ##.#.... */
	/*02be:*/	0xd0,  /* ##.#.... */
	/*02bf:*/	0xd0,  /* ##.#.... */
	/*02c0:*/	0xe0,  /* ###..... */
/* --- new character c (99) starting at offset 0x02c1 --- */
	/*02c1:*/	4, 3, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*02c6:*/	0x60,  /* .##..... */
	/*02c7:*/	0xc0,  /* ##...... */
	/*02c8:*/	0xc0,  /* ##...... */
	/*02c9:*/	0xc0,  /* ##...... */
	/*02ca:*/	0x60,  /* .##..... */
/* --- new character d (100) starting at offset 0x02cb --- */
	/*02cb:*/	5, 4, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*02d0:*/	0x30,  /* ..##.... */
	/*02d1:*/	0x30,  /* ..##.... */
	/*02d2:*/	0x70,  /* .###.... */
	/*02d3:*/	0xb0,  /* #.##.... */
	/*02d4:*/	0xb0,  /* #.##.... */
	/*02d5:*/	0xb0,  /* #.##.... */
	/*02d6:*/	0x70,  /* .###.... */
/* --- new character e (101) starting at offset 0x02d7 --- */
	/*02d7:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*02dc:*/	0x60,  /* .##..... */
	/*02dd:*/	0xd0,  /* ##.#.... */
	/*02de:*/	0xf0,  /* ####.... */
	/*02df:*/	0xc0,  /* ##...... */
	/*02e0:*/	0x60,  /* .##..... */
/* --- new character f (102) starting at offset 0x02e1 --- */
	/*02e1:*/	3, 4, 7, -1, 0, /* width and bbox (w,h,x,y) */ 
	/*02e6:*/	0x30,  /* ..##.... */
	/*02e7:*/	0x60,  /* .##..... */
	/*02e8:*/	0xf0,  /* ####.... */
	/*02e9:*/	0x60,  /* .##..... */
	/*02ea:*/	0x60,  /* .##..... */
	/*02eb:*/	0x60,  /* .##..... */
	/*02ec:*/	0x60,  /* .##..... */
/* --- new character g (103) starting at offset 0x02ed --- */
	/*02ed:*/	5, 4, 6, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*02f2:*/	0xd0,  /* ##.#.... */
	/*02f3:*/	0xb0,  /* #.##.... */
	/*02f4:*/	0xb0,  /* #.##.... */
	/*02f5:*/	0xf0,  /* ####.... */
	/*02f6:*/	0x30,  /* ..##.... */
	/*02f7:*/	0xe0,  /* ###..... */
/* --- new character h (104) starting at offset 0x02f8 --- */
	/*02f8:*/	5, 4, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*02fd:*/	0xc0,  /* ##...... */
	/*02fe:*/	0xc0,  /* ##...... */
	/*02ff:*/	0xe0,  /* ###..... */
	/*0300:*/	0xd0,  /* ##.#.... */
	/*0301:*/	0xd0,  /* ##.#.... */
	/*0302:*/	0xd0,  /* ##.#.... */
	/*0303:*/	0xd0,  /* ##.#.... */
/* --- new character i (105) starting at offset 0x0304 --- */
	/*0304:*/	2, 1, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0309:*/	0x80,  /* #....... */
	/*030a:*/	0x00,  /* ........ */
	/*030b:*/	0x80,  /* #....... */
	/*030c:*/	0x80,  /* #....... */
	/*030d:*/	0x80,  /* #....... */
	/*030e:*/	0x80,  /* #....... */
	/*030f:*/	0x80,  /* #....... */
/* --- new character j (106) starting at offset 0x0310 --- */
	/*0310:*/	2, 2, 8, -1, -1, /* width and bbox (w,h,x,y) */ 
	/*0315:*/	0x40,  /* .#...... */
	/*0316:*/	0x00,  /* ........ */
	/*0317:*/	0x40,  /* .#...... */
	/*0318:*/	0x40,  /* .#...... */
	/*0319:*/	0x40,  /* .#...... */
	/*031a:*/	0x40,  /* .#...... */
	/*031b:*/	0x40,  /* .#...... */
	/*031c:*/	0x80,  /* #....... */
/* --- new character k (107) starting at offset 0x031d --- */
	/*031d:*/	5, 4, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0322:*/	0xc0,  /* ##...... */
	/*0323:*/	0xc0,  /* ##...... */
	/*0324:*/	0xd0,  /* ##.#.... */
	/*0325:*/	0xd0,  /* ##.#.... */
	/*0326:*/	0xe0,  /* ###..... */
	/*0327:*/	0xd0,  /* ##.#.... */
	/*0328:*/	0xd0,  /* ##.#.... */
/* --- new character l (108) starting at offset 0x0329 --- */
	/*0329:*/	2, 1, 7, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*032e:*/	0x80,  /* #....... */
	/*032f:*/	0x80,  /* #....... */
	/*0330:*/	0x80,  /* #....... */
	/*0331:*/	0x80,  /* #....... */
	/*0332:*/	0x80,  /* #....... */
	/*0333:*/	0x80,  /* #....... */
	/*0334:*/	0x80,  /* #....... */
/* --- new character m (109) starting at offset 0x0335 --- */
	/*0335:*/	7, 6, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*033a:*/	0xe8,  /* ###.#... */
	/*033b:*/	0xd4,  /* ##.#.#.. */
	/*033c:*/	0xd4,  /* ##.#.#.. */
	/*033d:*/	0xd4,  /* ##.#.#.. */
	/*033e:*/	0xd4,  /* ##.#.#.. */
/* --- new character n (110) starting at offset 0x033f --- */
	/*033f:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0344:*/	0xe0,  /* ###..... */
	/*0345:*/	0xd0,  /* ##.#.... */
	/*0346:*/	0xd0,  /* ##.#.... */
	/*0347:*/	0xd0,  /* ##.#.... */
	/*0348:*/	0xd0,  /* ##.#.... */
/* --- new character o (111) starting at offset 0x0349 --- */
	/*0349:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*034e:*/	0x60,  /* .##..... */
	/*034f:*/	0xd0,  /* ##.#.... */
	/*0350:*/	0xd0,  /* ##.#.... */
	/*0351:*/	0xd0,  /* ##.#.... */
	/*0352:*/	0x60,  /* .##..... */
/* --- new character p (112) starting at offset 0x0353 --- */
	/*0353:*/	5, 4, 6, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*0358:*/	0xe0,  /* ###..... */
	/*0359:*/	0xd0,  /* ##.#.... */
	/*035a:*/	0xd0,  /* ##.#.... */
	/*035b:*/	0xd0,  /* ##.#.... */
	/*035c:*/	0xe0,  /* ###..... */
	/*035d:*/	0xc0,  /* ##...... */
/* --- new character q (113) starting at offset 0x035e --- */
	/*035e:*/	5, 4, 6, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*0363:*/	0x70,  /* .###.... */
	/*0364:*/	0xb0,  /* #.##.... */
	/*0365:*/	0xb0,  /* #.##.... */
	/*0366:*/	0xb0,  /* #.##.... */
	/*0367:*/	0x70,  /* .###.... */
	/*0368:*/	0x30,  /* ..##.... */
/* --- new character r (114) starting at offset 0x0369 --- */
	/*0369:*/	3, 3, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*036e:*/	0xa0,  /* #.#..... */
	/*036f:*/	0xe0,  /* ###..... */
	/*0370:*/	0xc0,  /* ##...... */
	/*0371:*/	0xc0,  /* ##...... */
	/*0372:*/	0xc0,  /* ##...... */
/* --- new character s (115) starting at offset 0x0373 --- */
	/*0373:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0378:*/	0x70,  /* .###.... */
	/*0379:*/	0xc0,  /* ##...... */
	/*037a:*/	0xf0,  /* ####.... */
	/*037b:*/	0x30,  /* ..##.... */
	/*037c:*/	0xe0,  /* ###..... */
/* --- new character t (116) starting at offset 0x037d --- */
	/*037d:*/	3, 4, 7, -1, 0, /* width and bbox (w,h,x,y) */ 
	/*0382:*/	0x20,  /* ..#..... */
	/*0383:*/	0x60,  /* .##..... */
	/*0384:*/	0xf0,  /* ####.... */
	/*0385:*/	0x60,  /* .##..... */
	/*0386:*/	0x60,  /* .##..... */
	/*0387:*/	0x60,  /* .##..... */
	/*0388:*/	0x30,  /* ..##.... */
/* --- new character u (117) starting at offset 0x0389 --- */
	/*0389:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*038e:*/	0xd0,  /* ##.#.... */
	/*038f:*/	0xd0,  /* ##.#.... */
	/*0390:*/	0xd0,  /* ##.#.... */
	/*0391:*/	0xf0,  /* ####.... */
	/*0392:*/	0x50,  /* .#.#.... */
/* --- new character v (118) starting at offset 0x0393 --- */
	/*0393:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*0398:*/	0xd0,  /* ##.#.... */
	/*0399:*/	0xd0,  /* ##.#.... */
	/*039a:*/	0xd0,  /* ##.#.... */
	/*039b:*/	0x60,  /* .##..... */
	/*039c:*/	0x40,  /* .#...... */
/* --- new character w (119) starting at offset 0x039d --- */
	/*039d:*/	6, 5, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*03a2:*/	0xa8,  /* #.#.#... */
	/*03a3:*/	0xa8,  /* #.#.#... */
	/*03a4:*/	0xf8,  /* #####... */
	/*03a5:*/	0xf8,  /* #####... */
	/*03a6:*/	0x48,  /* .#..#... */
/* --- new character x (120) starting at offset 0x03a7 --- */
	/*03a7:*/	6, 5, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*03ac:*/	0xd8,  /* ##.##... */
	/*03ad:*/	0xd8,  /* ##.##... */
	/*03ae:*/	0x70,  /* .###.... */
	/*03af:*/	0xd8,  /* ##.##... */
	/*03b0:*/	0xd8,  /* ##.##... */
/* --- new character y (121) starting at offset 0x03b1 --- */
	/*03b1:*/	5, 4, 6, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*03b6:*/	0xd0,  /* ##.#.... */
	/*03b7:*/	0xd0,  /* ##.#.... */
	/*03b8:*/	0xd0,  /* ##.#.... */
	/*03b9:*/	0x70,  /* .###.... */
	/*03ba:*/	0x60,  /* .##..... */
	/*03bb:*/	0x60,  /* .##..... */
/* --- new character z (122) starting at offset 0x03bc --- */
	/*03bc:*/	5, 4, 5, 0, 0, /* width and bbox (w,h,x,y) */ 
	/*03c1:*/	0xf0,  /* ####.... */
	/*03c2:*/	0x30,  /* ..##.... */
	/*03c3:*/	0x60,  /* .##..... */
	/*03c4:*/	0xc0,  /* ##...... */
	/*03c5:*/	0xf0,  /* ####.... */
/* --- new character braceleft (123) starting at offset 0x03c6 --- */
	/*03c6:*/	4, 3, 7, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*03cb:*/	0x20,  /* ..#..... */
	/*03cc:*/	0x40,  /* .#...... */
	/*03cd:*/	0x40,  /* .#...... */
	/*03ce:*/	0x80,  /* #....... */
	/*03cf:*/	0x40,  /* .#...... */
	/*03d0:*/	0x40,  /* .#...... */
	/*03d1:*/	0x20,  /* ..#..... */
/* --- new character bar (124) starting at offset 0x03d2 --- */
	/*03d2:*/	2, 1, 7, 1, -1, /* width and bbox (w,h,x,y) */ 
	/*03d7:*/	0x80,  /* #....... */
	/*03d8:*/	0x80,  /* #....... */
	/*03d9:*/	0x80,  /* #....... */
	/*03da:*/	0x80,  /* #....... */
	/*03db:*/	0x80,  /* #....... */
	/*03dc:*/	0x80,  /* #....... */
	/*03dd:*/	0x80,  /* #....... */
/* --- new character braceright (125) starting at offset 0x03de --- */
	/*03de:*/	4, 3, 7, 0, -1, /* width and bbox (w,h,x,y) */ 
	/*03e3:*/	0x80,  /* #....... */
	/*03e4:*/	0x40,  /* .#...... */
	/*03e5:*/	0x40,  /* .#...... */
	/*03e6:*/	0x20,  /* ..#..... */
	/*03e7:*/	0x40,  /* .#...... */
	/*03e8:*/	0x40,  /* .#...... */
	/*03e9:*/	0x80,  /* #....... */
/* --- new character asciitilde (126) starting at offset 0x03ea --- */
	/*03ea:*/	5, 5, 2, 0, 2, /* width and bbox (w,h,x,y) */ 
	/*03ef:*/	0x58,  /* .#.##... */
	/*03f0:*/	0xb0,  /* #.##.... */
};
static const uint16_t font_helvB08_offsets[] = {
0x0000 /* space */,
	0x0006 /* exclam */,
	0x0012 /* quotedbl */,
	0x0019 /* numbersign */,
	0x0024 /* dollar */,
	0x0031 /* percent */,
	0x003c /* ampersand */,
	0x0047 /* quotesingle */,
	0x004f /* parenleft */,
	0x005c /* parenright */,
	0x0069 /* asterisk */,
	0x0071 /* plus */,
	0x007b /* comma */,
	0x0083 /* hyphen */,
	0x0089 /* period */,
	0x0090 /* slash */,
	0x009b /* zero */,
	0x00a6 /* one */,
	0x00b1 /* two */,
	0x00bc /* three */,
	0x00c7 /* four */,
	0x00d2 /* five */,
	0x00dd /* six */,
	0x00e8 /* seven */,
	0x00f3 /* eight */,
	0x00fe /* nine */,
	0x0109 /* colon */,
	0x0113 /* semicolon */,
	0x011e /* less */,
	0x0128 /* equal */,
	0x0130 /* greater */,
	0x013a /* question */,
	0x0146 /* at */,
	0x0152 /* A */,
	0x015d /* B */,
	0x0168 /* C */,
	0x0173 /* D */,
	0x017e /* E */,
	0x0189 /* F */,
	0x0194 /* G */,
	0x019f /* H */,
	0x01aa /* I */,
	0x01b5 /* J */,
	0x01c0 /* K */,
	0x01cb /* L */,
	0x01d6 /* M */,
	0x01e1 /* N */,
	0x01ec /* O */,
	0x01f7 /* P */,
	0x0202 /* Q */,
	0x020e /* R */,
	0x0219 /* S */,
	0x0224 /* T */,
	0x022f /* U */,
	0x023a /* V */,
	0x0245 /* W */,
	0x0250 /* X */,
	0x025b /* Y */,
	0x0266 /* Z */,
	0x0271 /* bracketleft */,
	0x027e /* backslash */,
	0x0289 /* bracketright */,
	0x0296 /* asciicircum */,
	0x029e /* underscore */,
	0x02a4 /* grave */,
	0x02ab /* a */,
	0x02b5 /* b */,
	0x02c1 /* c */,
	0x02cb /* d */,
	0x02d7 /* e */,
	0x02e1 /* f */,
	0x02ed /* g */,
	0x02f8 /* h */,
	0x0304 /* i */,
	0x0310 /* j */,
	0x031d /* k */,
	0x0329 /* l */,
	0x0335 /* m */,
	0x033f /* n */,
	0x0349 /* o */,
	0x0353 /* p */,
	0x035e /* q */,
	0x0369 /* r */,
	0x0373 /* s */,
	0x037d /* t */,
	0x0389 /* u */,
	0x0393 /* v */,
	0x039d /* w */,
	0x03a7 /* x */,
	0x03b1 /* y */,
	0x03bc /* z */,
	0x03c6 /* braceleft */,
	0x03d2 /* bar */,
	0x03de /* braceright */,
	0x03ea /* asciitilde */,
	0xffff /* (no glyph) */
};
const struct fb_font font_helvB08 = {
	.height = 10,
	.ascent = 8,
	.firstchar = 32, /* space */
	.lastchar = 127, /* ? */
	.chardata = font_helvB08_data,
	.charoffs = font_helvB08_offsets,
};
