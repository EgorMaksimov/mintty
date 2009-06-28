// unicode.c (part of MinTTY)
// Copyright 2008-09  Andy Koppe
// Based on code from PuTTY-0.60 by Simon Tatham and team.
// Licensed under the terms of the GNU General Public License v3 or later.

#include "unicode.h"

#include "config.h"

#include <winbase.h>
#include <winnls.h>

unicode_data ucsdata;

/* Character conversion arrays; they are usually taken from windows,
 * the xterm one has the four scanlines that have no unicode 2.0
 * equivalents mapped to their unicode 3.0 locations.
 */
static const wchar unitab_xterm_std[32] = {
  0x2666, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0, 0x00b1,
  0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0x23ba,
  0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524, 0x2534, 0x252c,
  0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3, 0x00b7, 0x0020
};

/*
 * If the codepage is non-zero it's a window codepage, zero means use a
 * local codepage. The name is always converted to the first of any
 * duplicate definitions.
 */

/* 
 * Tables for ISO-8859-{1-10,13-16} derived from those downloaded
 * 2001-10-02 from <http://www.unicode.org/Public/MAPPINGS/> -- jtn
 * Table for ISO-8859-11 derived from same on 2002-11-18. -- bjh21
 */

/* XXX: This could be done algorithmically, but I'm not sure it's
 *      worth the hassle -- jtn */
/* ISO/IEC 8859-1:1998 (Latin-1, "Western", "West European") */
static const wchar iso_8859_1[] = {
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
  0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
  0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
  0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
  0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
  0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
  0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

/* ISO/IEC 8859-2:1999 (Latin-2, "Central European", "East European") */
static const wchar iso_8859_2[] = {
  0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7,
  0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
  0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7,
  0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
  0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
  0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
  0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
  0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
  0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
  0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
  0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
  0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
};

/* ISO/IEC 8859-3:1999 (Latin-3, "South European", "Maltese & Esperanto") */
static const wchar iso_8859_3[] = {
  0x00A0, 0x0126, 0x02D8, 0x00A3, 0x00A4, 0xFFFD, 0x0124, 0x00A7,
  0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD, 0xFFFD, 0x017B,
  0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7,
  0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD, 0xFFFD, 0x017C,
  0x00C0, 0x00C1, 0x00C2, 0xFFFD, 0x00C4, 0x010A, 0x0108, 0x00C7,
  0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0xFFFD, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x0120, 0x00D6, 0x00D7,
  0x011C, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x016C, 0x015C, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0xFFFD, 0x00E4, 0x010B, 0x0109, 0x00E7,
  0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0xFFFD, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7,
  0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9
};

/* ISO/IEC 8859-4:1998 (Latin-4, "North European") */
static const wchar iso_8859_4[] = {
  0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B, 0x00A7,
  0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF,
  0x00B0, 0x0105, 0x02DB, 0x0157, 0x00B4, 0x0129, 0x013C, 0x02C7,
  0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B,
  0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E,
  0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x012A,
  0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
  0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF,
  0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
  0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B,
  0x0111, 0x0146, 0x014D, 0x0137, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
  0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9
};

/* ISO/IEC 8859-5:1999 (Latin/Cyrillic) */
static const wchar iso_8859_5[] = {
  0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
  0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x00AD, 0x040E, 0x040F,
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
  0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
  0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
  0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
  0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
  0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457,
  0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F
};

/* ISO/IEC 8859-6:1999 (Latin/Arabic) */
static const wchar iso_8859_6[] = {
  0x00A0, 0xFFFD, 0xFFFD, 0xFFFD, 0x00A4, 0xFFFD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x060C, 0x00AD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0x061B, 0xFFFD, 0xFFFD, 0xFFFD, 0x061F,
  0xFFFD, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627,
  0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
  0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637,
  0x0638, 0x0639, 0x063A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647,
  0x0648, 0x0649, 0x064A, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F,
  0x0650, 0x0651, 0x0652, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD
};

/* ISO 8859-7:1987 (Latin/Greek) */
static const wchar iso_8859_7[] = {
  0x00A0, 0x2018, 0x2019, 0x00A3, 0xFFFD, 0xFFFD, 0x00A6, 0x00A7,
  0x00A8, 0x00A9, 0xFFFD, 0x00AB, 0x00AC, 0x00AD, 0xFFFD, 0x2015,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7,
  0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
  0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
  0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
  0x03A0, 0x03A1, 0xFFFD, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
  0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
  0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
  0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
  0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
  0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0xFFFD
};

/* ISO/IEC 8859-8:1999 (Latin/Hebrew) */
static const wchar iso_8859_8[] = {
  0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
  0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
  0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x2017,
  0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
  0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
  0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
  0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0x200E, 0x200F, 0xFFFD
};

/* ISO/IEC 8859-9:1999 (Latin-5, "Turkish") */
static const wchar iso_8859_9[] = {
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
  0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
  0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
  0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
  0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
  0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
  0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF
};

/* ISO/IEC 8859-10:1998 (Latin-6, "Nordic" [Sami, Inuit, Icelandic]) */
static const wchar iso_8859_10[] = {
  0x00A0, 0x0104, 0x0112, 0x0122, 0x012A, 0x0128, 0x0136, 0x00A7,
  0x013B, 0x0110, 0x0160, 0x0166, 0x017D, 0x00AD, 0x016A, 0x014A,
  0x00B0, 0x0105, 0x0113, 0x0123, 0x012B, 0x0129, 0x0137, 0x00B7,
  0x013C, 0x0111, 0x0161, 0x0167, 0x017E, 0x2015, 0x016B, 0x014B,
  0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E,
  0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x00CF,
  0x00D0, 0x0145, 0x014C, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0168,
  0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
  0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
  0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x00EF,
  0x00F0, 0x0146, 0x014D, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0169,
  0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0138
};

/* ISO/IEC 8859-11:2001 ("Thai", "TIS620") */
static const wchar iso_8859_11[] = {
  0x00A0, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07,
  0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
  0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17,
  0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
  0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
  0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
  0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37,
  0x0E38, 0x0E39, 0x0E3A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x0E3F,
  0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47,
  0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
  0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57,
  0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD
};

/* ISO/IEC 8859-13:1998 (Latin-7, "Baltic Rim") */
static const wchar iso_8859_13[] = {
  0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6, 0x00A7,
  0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x201C, 0x00B5, 0x00B6, 0x00B7,
  0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
  0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
  0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
  0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
  0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
  0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
  0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
  0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
  0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019
};

/* ISO/IEC 8859-14:1998 (Latin-8, "Celtic", "Gaelic/Welsh") */
static const wchar iso_8859_14[] = {
  0x00A0, 0x1E02, 0x1E03, 0x00A3, 0x010A, 0x010B, 0x1E0A, 0x00A7,
  0x1E80, 0x00A9, 0x1E82, 0x1E0B, 0x1EF2, 0x00AD, 0x00AE, 0x0178,
  0x1E1E, 0x1E1F, 0x0120, 0x0121, 0x1E40, 0x1E41, 0x00B6, 0x1E56,
  0x1E81, 0x1E57, 0x1E83, 0x1E60, 0x1EF3, 0x1E84, 0x1E85, 0x1E61,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
  0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x0174, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x1E6A,
  0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x0176, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
  0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x0175, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x1E6B,
  0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x0177, 0x00FF
};

/* ISO/IEC 8859-15:1999 (Latin-9 aka -0, "euro") */
static const wchar iso_8859_15[] = {
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AC, 0x00A5, 0x0160, 0x00A7,
  0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x017D, 0x00B5, 0x00B6, 0x00B7,
  0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
  0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
  0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
  0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
  0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

/* ISO/IEC 8859-16:2001 (Latin-10, "Balkan") */
static const wchar iso_8859_16[] = {
  0x00A0, 0x0104, 0x0105, 0x0141, 0x20AC, 0x201E, 0x0160, 0x00A7,
  0x0161, 0x00A9, 0x0218, 0x00AB, 0x0179, 0x00AD, 0x017A, 0x017B,
  0x00B0, 0x00B1, 0x010C, 0x0142, 0x017D, 0x201D, 0x00B6, 0x00B7,
  0x017E, 0x010D, 0x0219, 0x00BB, 0x0152, 0x0153, 0x0178, 0x017C,
  0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0106, 0x00C6, 0x00C7,
  0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x0110, 0x0143, 0x00D2, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x015A,
  0x0170, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0118, 0x021A, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x0107, 0x00E6, 0x00E7,
  0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x0111, 0x0144, 0x00F2, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x015B,
  0x0171, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0119, 0x021B, 0x00FF
};

static const wchar roman8[] = {
  0x00A0, 0x00C0, 0x00C2, 0x00C8, 0x00CA, 0x00CB, 0x00CE, 0x00CF,
  0x00B4, 0x02CB, 0x02C6, 0x00A8, 0x02DC, 0x00D9, 0x00DB, 0x20A4,
  0x00AF, 0x00DD, 0x00FD, 0x00B0, 0x00C7, 0x00E7, 0x00D1, 0x00F1,
  0x00A1, 0x00BF, 0x00A4, 0x00A3, 0x00A5, 0x00A7, 0x0192, 0x00A2,
  0x00E2, 0x00EA, 0x00F4, 0x00FB, 0x00E1, 0x00E9, 0x00F3, 0x00FA,
  0x00E0, 0x00E8, 0x00F2, 0x00F9, 0x00E4, 0x00EB, 0x00F6, 0x00FC,
  0x00C5, 0x00EE, 0x00D8, 0x00C6, 0x00E5, 0x00ED, 0x00F8, 0x00E6,
  0x00C4, 0x00EC, 0x00D6, 0x00DC, 0x00C9, 0x00EF, 0x00DF, 0x00D4,
  0x00C1, 0x00C3, 0x00E3, 0x00D0, 0x00F0, 0x00CD, 0x00CC, 0x00D3,
  0x00D2, 0x00D5, 0x00F5, 0x0160, 0x0161, 0x00DA, 0x0178, 0x00FF,
  0x00DE, 0x00FE, 0x00B7, 0x00B5, 0x00B6, 0x00BE, 0x2014, 0x00BC,
  0x00BD, 0x00AA, 0x00BA, 0x00AB, 0x25A0, 0x00BB, 0x00B1, 0xFFFD
};

static const wchar koi8_u[] = {
  0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524,
  0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590,
  0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2022, 0x221A, 0x2248,
  0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7,
  0x2550, 0x2551, 0x2552, 0x0451, 0x0454, 0x2554, 0x0456, 0x0457,
  0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x0491, 0x255D, 0x255E,
  0x255F, 0x2560, 0x2561, 0x0401, 0x0404, 0x2563, 0x0406, 0x0407,
  0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x0490, 0x256C, 0x00A9,
  0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
  0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
  0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
  0x044C, 0x044B, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x044A,
  0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
  0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
  0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
  0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A
};

static const wchar vscii[] = {
  0x0000, 0x0001, 0x1EB2, 0x0003, 0x0004, 0x1EB4, 0x1EAA, 0x0007,
  0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
  0x0010, 0x0011, 0x0012, 0x0013, 0x1EF6, 0x0015, 0x0016, 0x0017,
  0x0018, 0x1EF8, 0x001a, 0x001b, 0x001c, 0x001d, 0x1EF4, 0x001f,
  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
  0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
  0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
  0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
  0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
  0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
  0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007f,
  0x1EA0, 0x1EAE, 0x1EB0, 0x1EB6, 0x1EA4, 0x1EA6, 0x1EA8, 0x1EAC,
  0x1EBC, 0x1EB8, 0x1EBE, 0x1EC0, 0x1EC2, 0x1EC4, 0x1EC6, 0x1ED0,
  0x1ED2, 0x1ED4, 0x1ED6, 0x1ED8, 0x1EE2, 0x1EDA, 0x1EDC, 0x1EDE,
  0x1ECA, 0x1ECE, 0x1ECC, 0x1EC8, 0x1EE6, 0x0168, 0x1EE4, 0x1EF2,
  0x00D5, 0x1EAF, 0x1EB1, 0x1EB7, 0x1EA5, 0x1EA7, 0x1EA8, 0x1EAD,
  0x1EBD, 0x1EB9, 0x1EBF, 0x1EC1, 0x1EC3, 0x1EC5, 0x1EC7, 0x1ED1,
  0x1ED3, 0x1ED5, 0x1ED7, 0x1EE0, 0x01A0, 0x1ED9, 0x1EDD, 0x1EDF,
  0x1ECB, 0x1EF0, 0x1EE8, 0x1EEA, 0x1EEC, 0x01A1, 0x1EDB, 0x01AF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x1EA2, 0x0102, 0x1EB3, 0x1EB5,
  0x00C8, 0x00C9, 0x00CA, 0x1EBA, 0x00CC, 0x00CD, 0x0128, 0x1EF3,
  0x0110, 0x1EE9, 0x00D2, 0x00D3, 0x00D4, 0x1EA1, 0x1EF7, 0x1EEB,
  0x1EED, 0x00D9, 0x00DA, 0x1EF9, 0x1EF5, 0x00DD, 0x1EE1, 0x01B0,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x1EA3, 0x0103, 0x1EEF, 0x1EAB,
  0x00E8, 0x00E9, 0x00EA, 0x1EBB, 0x00EC, 0x00ED, 0x0129, 0x1EC9,
  0x0111, 0x1EF1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x1ECF, 0x1ECD,
  0x1EE5, 0x00F9, 0x00FA, 0x0169, 0x1EE7, 0x00FD, 0x1EE3, 0x1EEE
};

static const wchar dec_mcs[] = {
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0xFFFD, 0x00A5, 0xFFFD, 0x00A7,
  0x00A4, 0x00A9, 0x00AA, 0x00AB, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0xFFFD, 0x00B5, 0x00B6, 0x00B7,
  0xFFFD, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0xFFFD, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
  0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0xFFFD, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0152,
  0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0178, 0xFFFD, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
  0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0xFFFD, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0153,
  0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FF, 0xFFFD, 0xFFFD
};

/* Mazovia (Polish) aka CP620
 * from "Mazowia to Unicode table", 04/24/96, Mikolaj Jedrzejak */
static const wchar mazovia[] = {
 /* Code point 0x9B is "zloty" symbol (z&#0142;), which is not
  *   widely used and for which there is no Unicode equivalent.
  * One reference shows 0xA8 as U+00A7 SECTION SIGN, but we're
  *   told that's incorrect. */
  0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x0105, 0x00E7,
  0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x0107, 0x00C4, 0x0104,
  0x0118, 0x0119, 0x0142, 0x00F4, 0x00F6, 0x0106, 0x00FB, 0x00F9,
  0x015a, 0x00D6, 0x00DC, 0xFFFD, 0x0141, 0x00A5, 0x015b, 0x0192,
  0x0179, 0x017b, 0x00F3, 0x00d3, 0x0144, 0x0143, 0x017a, 0x017c,
  0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
  0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
  0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
  0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
  0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
  0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
  0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
  0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
  0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
  0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

struct cp_list_item {
  char *name;
  int codepage;
  int cp_size;
  const wchar *cp_table;
};

static const struct cp_list_item cp_list[] = {
  {"ISO-8859-1:1998 (Latin-1, West Europe)", 0, 96, iso_8859_1},
  {"ISO-8859-2:1999 (Latin-2, East Europe)", 0, 96, iso_8859_2},
  {"ISO-8859-3:1999 (Latin-3, South Europe)", 0, 96, iso_8859_3},
  {"ISO-8859-4:1998 (Latin-4, North Europe)", 0, 96, iso_8859_4},
  {"ISO-8859-5:1999 (Latin/Cyrillic)", 0, 96, iso_8859_5},
  {"ISO-8859-6:1999 (Latin/Arabic)", 0, 96, iso_8859_6},
  {"ISO-8859-7:1987 (Latin/Greek)", 0, 96, iso_8859_7},
  {"ISO-8859-8:1999 (Latin/Hebrew)", 0, 96, iso_8859_8},
  {"ISO-8859-9:1999 (Latin-5, Turkish)", 0, 96, iso_8859_9},
  {"ISO-8859-10:1998 (Latin-6, Nordic)", 0, 96, iso_8859_10},
  {"ISO-8859-11:2001 (Latin/Thai)", 0, 96, iso_8859_11},
  {"ISO-8859-13:1998 (Latin-7, Baltic)", 0, 96, iso_8859_13},
  {"ISO-8859-14:1998 (Latin-8, Celtic)", 0, 96, iso_8859_14},
  {"ISO-8859-15:1999 (Latin-9, \"euro\")", 0, 96, iso_8859_15},
  {"ISO-8859-16:2001 (Latin-10, Balkan)", 0, 96, iso_8859_16},

  {"KOI8-U", 0, 128, koi8_u},
  {"KOI8-R", 20866, 0, 0},
  {"HP-ROMAN8", 0, 96, roman8},
  {"VSCII", 0, 256, vscii},
  {"DEC-MCS", 0, 96, dec_mcs},

  {"UTF-8", CP_UTF8, 0, 0},

  {"Win1250 (Central European)", 1250, 0, 0},
  {"Win1251 (Cyrillic)", 1251, 0, 0},
  {"Win1252 (Western)", 1252, 0, 0},
  {"Win1253 (Greek)", 1253, 0, 0},
  {"Win1254 (Turkish)", 1254, 0, 0},
  {"Win1255 (Hebrew)", 1255, 0, 0},
  {"Win1256 (Arabic)", 1256, 0, 0},
  {"Win1257 (Baltic)", 1257, 0, 0},
  {"Win1258 (Vietnamese)", 1258, 0, 0},

  {"CP437", 437, 0, 0},
  {"CP620 (Mazovia)", 0, 128, mazovia},
  {"CP819", 28591, 0, 0},
  {"CP878", 20866, 0, 0},

  {"Use font encoding", -1, 0, 0},

  {0, 0, 0, 0}
};

static void link_font(wchar * line_tbl, wchar * font_tbl, wchar attr);

void
init_ucs(void)
{
  int i, j;
  int used_dtf = 0;
  char tbuf[256];

  for (i = 0; i < 256; i++)
    tbuf[i] = i;

 /* Decide on the Line and Font codepages */
  ucsdata.codepage = decode_codepage(cfg.codepage);

  if (ucsdata.font_codepage <= 0) {
    ucsdata.font_codepage = 0;
    ucsdata.dbcs_screenfont = 0;
  }

  if (ucsdata.codepage <= 0)
    ucsdata.codepage = ucsdata.font_codepage;

 /* Collect screen font ucs table */
  if (ucsdata.dbcs_screenfont || ucsdata.font_codepage == 0) {
    get_unitab(ucsdata.font_codepage, ucsdata.unitab_font, 2);
    for (i = 128; i < 256; i++)
      ucsdata.unitab_font[i] = (wchar) (CSET_ACP + i);
  }
  else {
    get_unitab(ucsdata.font_codepage, ucsdata.unitab_font, 1);

   /* CP437 fonts are often broken ... */
    if (ucsdata.font_codepage == 437)
      ucsdata.unitab_font[0] = ucsdata.unitab_font[255] = 0xFFFF;
  }

 /* Collect OEMCP ucs table */
  get_unitab(CP_OEMCP, ucsdata.unitab_oemcp, 1);

  get_unitab(437, ucsdata.unitab_scoacs, 1);

 /* Collect line set ucs table */
  if (ucsdata.codepage == ucsdata.font_codepage &&
      (ucsdata.dbcs_screenfont || ucsdata.font_codepage == 0)) {

   /* For DBCS and POOR fonts force direct to font */
    used_dtf = 1;
    for (i = 0; i < 32; i++)
      ucsdata.unitab_line[i] = (wchar) i;
    for (i = 32; i < 256; i++)
      ucsdata.unitab_line[i] = (wchar) (CSET_ACP + i);
    ucsdata.unitab_line[127] = (wchar) 127;
  }
  else
    get_unitab(ucsdata.codepage, ucsdata.unitab_line, 0);

 /* VT100 graphics - NB: Broken for non-ascii CP's */
  memcpy(ucsdata.unitab_xterm, ucsdata.unitab_line,
         sizeof (ucsdata.unitab_xterm));
  memcpy(ucsdata.unitab_xterm + '`', unitab_xterm_std,
         sizeof (unitab_xterm_std));
  ucsdata.unitab_xterm['_'] = ' ';

 /* Generate UCS ->line page table. */
  if (ucsdata.uni_tbl) {
    for (i = 0; i < 256; i++)
      if (ucsdata.uni_tbl[i])
        free(ucsdata.uni_tbl[i]);
    free(ucsdata.uni_tbl);
    ucsdata.uni_tbl = 0;
  }
  if (!used_dtf) {
    for (i = 0; i < 256; i++) {
      if (DIRECT_CHAR(ucsdata.unitab_line[i]))
        continue;
      if (DIRECT_FONT(ucsdata.unitab_line[i]))
        continue;
      if (!ucsdata.uni_tbl) {
        ucsdata.uni_tbl = newn(char *, 256);
        memset(ucsdata.uni_tbl, 0, 256 * sizeof (char *));
      }
      j = ((ucsdata.unitab_line[i] >> 8) & 0xFF);
      if (!ucsdata.uni_tbl[j]) {
        ucsdata.uni_tbl[j] = newn(char, 256);
        memset(ucsdata.uni_tbl[j], 0, 256 * sizeof (char));
      }
      ucsdata.uni_tbl[j][ucsdata.unitab_line[i] & 0xFF] = i;
    }
  }

 /* Find the line control characters. */
  for (i = 0; i < 256; i++)
    if (ucsdata.unitab_line[i] < ' ' ||
        (ucsdata.unitab_line[i] >= 0x7F && ucsdata.unitab_line[i] < 0xA0))
      ucsdata.unitab_ctrl[i] = i;
    else
      ucsdata.unitab_ctrl[i] = 0xFF;

  link_font(ucsdata.unitab_line, ucsdata.unitab_font, CSET_ACP);
  link_font(ucsdata.unitab_scoacs, ucsdata.unitab_font, CSET_ACP);
  link_font(ucsdata.unitab_xterm, ucsdata.unitab_font, CSET_ACP);

  if (ucsdata.dbcs_screenfont && ucsdata.font_codepage != ucsdata.codepage) {
   /* F***ing Microsoft fonts, Japanese and Korean codepage fonts
    * have a currency symbol at 0x5C but their unicode value is 
    * still given as U+005C not the correct U+00A5. */
    ucsdata.unitab_line['\\'] = CSET_OEMCP + '\\';
  }
}

static void
link_font(wchar * line_tbl, wchar * font_tbl, wchar attr)
{
  int font_index, line_index, i;
  for (line_index = 0; line_index < 256; line_index++) {
    if (DIRECT_FONT(line_tbl[line_index]))
      continue;
    for (i = 0; i < 256; i++) {
      font_index = ((32 + i) & 0xFF);
      if (line_tbl[line_index] == font_tbl[font_index]) {
        line_tbl[line_index] = (wchar) (attr + font_index);
        break;
      }
    }
  }
}

int
decode_codepage(char *cp_name)
{
  char *s, *d;
  const struct cp_list_item *cpi;
  int codepage = -1;
  CPINFO cpinfo;

  if (!*cp_name) {
   /*
    * Here we select a plausible default code page based on
    * the locale the user is in. We wish to select an ISO code
    * page or appropriate local default _rather_ than go with
    * the Win125* series, because it's more important to have
    * CSI and friends enabled by default than the ghastly
    * Windows extra quote characters, and because it's more
    * likely the user is connecting to a remote server that
    * does something Unixy or VMSy and hence standards-
    * compliant than that they're connecting back to a Windows
    * box using horrible nonstandard charsets.
    * 
    * Accordingly, Robert de Bath suggests a method for
    * picking a default character set that runs as follows:
    * first call GetACP to get the system's ANSI code page
    * identifier, and translate as follows.
    * 
    * For anything else, choose direct-to-font.
    */
    int cp = GetACP();
    switch (cp) {
      when 1250: cp_name = "ISO-8859-2";
      when 1251: cp_name = "KOI8-U";
      when 1252: cp_name = "ISO-8859-1";
      when 1253: cp_name = "ISO-8859-7";
      when 1254: cp_name = "ISO-8859-9";
      when 1255: cp_name = "ISO-8859-8";
      when 1256: cp_name = "ISO-8859-6";
      when 1257: cp_name = "ISO-8859-13";
       /* default: leave it blank, which will select -1, direct->font */
    }
  }

  if (cp_name && *cp_name)
    for (cpi = cp_list; cpi->name; cpi++) {
      s = cp_name;
      d = cpi->name;
      for (;;) {
        while (*s && !isalnum((uchar)*s) && *s != ':')
          s++;
        while (*d && !isalnum((uchar)*d) && *d != ':')
          d++;
        if (*s == 0) {
          codepage = cpi->codepage;
          if (codepage == CP_UTF8)
            goto break_break;
          if (codepage == -1)
            return codepage;
          if (codepage == 0) {
            codepage = 65536 + (cpi - cp_list);
            goto break_break;
          }

          if (GetCPInfo(codepage, &cpinfo) != 0)
            goto break_break;
        }
        if (tolower((uchar)*s++) != tolower((uchar)*d++))
          break;
      }
    }

  if (cp_name && *cp_name) {
    d = cp_name;
    if (tolower((uchar)d[0]) == 'c' && 
	    tolower((uchar)d[1]) == 'p')
      d += 2;
    if (tolower((uchar)d[0]) == 'i' && 
	    tolower((uchar)d[1]) == 'b' && 
	    tolower((uchar)d[2]) == 'm')
      d += 3;
    for (s = d; *s >= '0' && *s <= '9'; s++);
    if (*s == 0 && s != d)
      codepage = atoi(d);       /* CP999 or IBM999 */

    if (codepage == CP_ACP)
      codepage = GetACP();
    if (codepage == CP_OEMCP)
      codepage = GetOEMCP();
    if (codepage > 65535)
      codepage = -2;
  }

break_break:;
  if (codepage != -1) {
    if (codepage != CP_UTF8 && codepage < 65536) {
      if (GetCPInfo(codepage, &cpinfo) == 0) {
        codepage = -2;
      }
      else if (cpinfo.MaxCharSize > 1)
        codepage = -3;
    }
  }
  if (codepage == -1 && *cp_name)
    codepage = -2;
  return codepage;
}

const char *
cp_name(int codepage)
{
  const struct cp_list_item *cpi, *cpno;
  static char buf[32];

  if (codepage == -1) {
    sprintf(buf, "Use font encoding");
    return buf;
  }

  if (codepage > 0 && codepage < 65536)
    sprintf(buf, "CP%03d", codepage);
  else
    *buf = 0;

  if (codepage >= 65536) {
    cpno = 0;
    for (cpi = cp_list; cpi->name; cpi++)
      if (cpi == cp_list + (codepage - 65536)) {
        cpno = cpi;
        break;
      }
    if (cpno)
      for (cpi = cp_list; cpi->name; cpi++) {
        if (cpno->cp_table == cpi->cp_table)
          return cpi->name;
      }
  }
  else {
    for (cpi = cp_list; cpi->name; cpi++) {
      if (codepage == cpi->codepage)
        return cpi->name;
    }
  }
  return buf;
}

/*
 * Return the nth code page in the list, for use in the GUI
 * configurer.
 */
const char *
cp_enumerate(int index)
{
  if (index < 0 || index >= (int) lengthof(cp_list))
    return null;
  return cp_list[index].name;
}

void
get_unitab(int codepage, wchar * unitab, int ftype)
{
  char tbuf[4];
  int i, max = 256, flg = MB_ERR_INVALID_CHARS;

  if (ftype)
    flg |= MB_USEGLYPHCHARS;
  if (ftype == 2)
    max = 128;

  if (codepage == CP_UTF8) {
    for (i = 0; i < max; i++)
      unitab[i] = i;
    return;
  }

  if (codepage == CP_ACP)
    codepage = GetACP();
  else if (codepage == CP_OEMCP)
    codepage = GetOEMCP();

  if (codepage > 0 && codepage < 65536) {
    for (i = 0; i < max; i++) {
      tbuf[0] = i;

      if (mb_to_wc(codepage, flg, tbuf, 1, unitab + i, 1)
          != 1)
        unitab[i] = 0xFFFD;
    }
  }
  else {
    int j = 256 - cp_list[codepage & 0xFFFF].cp_size;
    for (i = 0; i < max; i++)
      unitab[i] = i;
    for (i = j; i < max; i++)
      unitab[i] = cp_list[codepage & 0xFFFF].cp_table[i - j];
  }
}

int
wc_to_mb(int codepage, int flags, const wchar * wcstr, int wclen,
                                  char *mbstr, int mblen)
{
  char *p;
  int i;
  if (codepage != unicode_codepage &&
      codepage == ucsdata.codepage && ucsdata.uni_tbl) {
   /* Do this by array lookup if we can. */
    if (wclen < 0) {
      for (wclen = 0; wcstr[wclen++];); /* will include the NUL */
    }
    for (p = mbstr, i = 0; i < wclen; i++) {
      wchar ch = wcstr[i];
      int by;
      char *p1;
      if (ucsdata.uni_tbl && (p1 = ucsdata.uni_tbl[(ch >> 8) & 0xFF])
          && (by = p1[ch & 0xFF]))
        *p++ = by;
      else if (ch < 0x80)
        *p++ = (char) ch;
      else
        *p++ = '.';
      assert(p - mbstr < mblen);
    }
    return p - mbstr;
  }
  else
    return WideCharToMultiByte(codepage, flags, wcstr, wclen, mbstr, mblen,
                               0, 0);
}

int
mb_to_wc(int codepage, int flags, const char *mbstr, int mblen,
                                  wchar * wcstr, int wclen)
{
  if (codepage > 0xffff) {
    // Custom codepage.
    if (codepage == ucsdata.codepage) {
      // It should be the configured codepage really.
      if (mblen < 0)
        mblen = strlen(mbstr);
      if (wclen <= 0)
        return mblen;
      if (wclen < mblen)
        mblen = wclen;
      for (int i = 0; i < mblen; i++)
        wcstr[i] = ucsdata.unitab_font[mbstr[i] & 0xff];
      return mblen;
    }
    else {
      // We shouldn't get here, but if we do anyway,
      // fall back to the ANSI codepage.
      codepage = GetACP();
    }
  }
  return MultiByteToWideChar(codepage, flags, mbstr, mblen, wcstr, wclen);
}

int
is_dbcs_leadbyte(int codepage, char byte)
{
  return IsDBCSLeadByteEx(codepage, byte);
}

int
wordtype(int c)
{
  struct ucsword {
    wchar start, end; 
    uchar ctype;
  };
  static const struct ucsword ucs_words[] = {
    {128, 160, 0},
    {161, 191, 1},
    {215, 215, 1},
    {247, 247, 1},
    {0x037e, 0x037e, 1},        /* Greek question mark */
    {0x0387, 0x0387, 1},        /* Greek ano teleia */
    {0x055a, 0x055f, 1},        /* Armenian punctuation */
    {0x0589, 0x0589, 1},        /* Armenian full stop */
    {0x0700, 0x070d, 1},        /* Syriac punctuation */
    {0x104a, 0x104f, 1},        /* Myanmar punctuation */
    {0x10fb, 0x10fb, 1},        /* Georgian punctuation */
    {0x1361, 0x1368, 1},        /* Ethiopic punctuation */
    {0x166d, 0x166e, 1},        /* Canadian Syl. punctuation */
    {0x17d4, 0x17dc, 1},        /* Khmer punctuation */
    {0x1800, 0x180a, 1},        /* Mongolian punctuation */
    {0x2000, 0x200a, 0},        /* Various spaces */
    {0x200b, 0x27ff, 1},        /* punctuation and symbols */
    {0x3000, 0x3000, 0},        /* ideographic space */
    {0x3001, 0x3020, 1},        /* ideographic punctuation */
    {0x303f, 0x309f, 3},        /* Hiragana */
    {0x30a0, 0x30ff, 3},        /* Katakana */
    {0x3300, 0x9fff, 3},        /* CJK Ideographs */
    {0xac00, 0xd7a3, 3},        /* Hangul Syllables */
    {0xf900, 0xfaff, 3},        /* CJK Ideographs */
    {0xfe30, 0xfe6b, 1},        /* punctuation forms */
    {0xff00, 0xff0f, 1},        /* half/fullwidth ASCII */
    {0xff1a, 0xff20, 1},        /* half/fullwidth ASCII */
    {0xff3b, 0xff40, 1},        /* half/fullwidth ASCII */
    {0xff5b, 0xff64, 1},        /* half/fullwidth ASCII */
    {0xfff0, 0xffff, 0},        /* half/fullwidth ASCII */
    {0, 0, 0}
  };
  switch (c & CSET_MASK) {
    when CSET_LINEDRW: c = ucsdata.unitab_xterm[c & 0xFF];
    when CSET_ASCII:   c = ucsdata.unitab_line[c & 0xFF];
    when CSET_SCOACS:  c = ucsdata.unitab_scoacs[c & 0xFF];
  }
  switch (c & CSET_MASK) {
    when CSET_ACP:   c = ucsdata.unitab_font[c & 0xFF];
    when CSET_OEMCP: c = ucsdata.unitab_oemcp[c & 0xFF];
  }

 /* For DBCS fonts I can't do anything useful. Even this will sometimes
  * fail as there's such a thing as a double width space. :-(
  */
  if (ucsdata.dbcs_screenfont && ucsdata.font_codepage == ucsdata.codepage)
    return (c != ' ');
  if (c < 0x80) {
    if (c <= ' ' || c == 0x7f)
      return 0;
    else if (isalnum(c) || strchr("#+-./\\_~", c))
      return 2;
    else
      return 1;
  }
  for (const struct ucsword *wptr = ucs_words; wptr->start; wptr++) {
    if (c >= wptr->start && c <= wptr->end)
      return wptr->ctype;
  }
  return 2;
}
