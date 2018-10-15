/* winkbd.h*/
/*
 * Copyright (c) 2000 Greg Haerr <greg@censoft.com>
 *
 * Win32 keyboard defines and API
 */

/* virtual key codes*/
#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03		/* ctrl-break*/
#define VK_MBUTTON        0x04
#define VK_BACK           0x08		/* backspace*/
#define VK_TAB            0x09
#define VK_CLEAR          0x0C		/* kp5 w/numlock off*/
#define VK_RETURN         0x0D
#define VK_SHIFT          0x10		/* either shift*/
#define VK_CONTROL        0x11		/* either control*/
#define VK_MENU           0x12		/* alt*/
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14		/* caps lock*/
#define VK_KANA           0x15
#define VK_HANGEUL        VK_KANA
#define VK_HANGUL         0x15
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19
#define VK_ESCAPE         0x1B		/* esc*/
#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F
#define VK_SPACE          0x20		/* spacebar*/
#define VK_PRIOR          0x21		/* page up*/
#define VK_NEXT           0x22		/* page dn*/
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

/* 0x30 - 0x39 ASCII 0 - 9*/
/* 0x41 - 0x5a ASCII A - Z*/

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

/* numeric keypad keys*/
#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A		/* kp * */
#define VK_ADD            0x6B		/* kp + */
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D		/* kp - */
#define VK_DECIMAL        0x6E		/* kp . */
#define VK_DIVIDE         0x6F		/* kp / */

#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87
#define VK_NUMLOCK        0x90		/* num lock*/
#define VK_SCROLL         0x91		/* scroll lock*/

/* param to GetAsyncKeyState and GetKeyState only*/
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#define VK_PROCESSKEY     0xE5
#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

/* WM_KEYUP/WM_KEYDOWN/WM_CHAR hiword lparam flags*/
#define KF_EXTENDED         0x0100
#define KF_DLGMODE          0x0800
#define KF_MENUMODE         0x1000
#define KF_ALTDOWN          0x2000
#define KF_REPEAT           0x4000
#define KF_UP               0x8000
