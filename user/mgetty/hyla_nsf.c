/* $Id: hyla_nsf.c,v 4.8 2010/06/05 09:49:37 gert Exp $ */
/* 
 * The tables in this file are taken from the HylaFAX distribution.  Thus,
 * the Hylafax copyright (below) applies, not the mgetty copyright (GPL).
 *
 * This file does not exist in the original HylaFAX distribution.
 * Created by Dmitry Bely, April 2000
 */
/*
 * Copyright (c) 1994-1996 Sam Leffler
 * Copyright (c) 1994-1996 Silicon Graphics, Inc.
 * HylaFAX is a trademark of Silicon Graphics
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */


#include <stdio.h>
#include <memory.h>
#include "mgetty.h"
#include "policy.h"

#ifdef FAX_NSF_PARSER

typedef char bool;
#define true 1
#define false 0

struct ModelData 
{
    const char* modelId;
    const char* modelName;
};

typedef struct ModelData ModelData;

struct NSFData {
    const char* vendorId;
    unsigned int vendorIdSize;	/* Country & provider code (T.35) */
    const char* vendorName;
    bool        inverseStationIdOrder;
    unsigned int         modelIdPos;
    unsigned int         modelIdSize;
    const ModelData* knownModels;
};

typedef struct NSFData NSFData;

/*
 * Unless a manufacturer is able to provide detailed specifics
 * of the construction of their NSF signals the only guaranteed
 * accurate information that we get from NSF is the manufacturer
 * (once the bitorder and construction of the first few bytes is
 * confirmed).  At that point we would ideally be able to identify
 * the model type, but doing that is often more guesswork than
 * anything and is more likely to prove wrong than right, depending
 * on how specific our ModelData typing is.  Any matches as to the
 * model identification really should be taken with some degree
 * of scepticism.
 *
 * As manufacturers often encode (in plain text) a station identification
 * in the NSF string it is often useful to look for that.
 */

static const ModelData Canon[] =
{{"\x80\x00\x80\x48\x00", "Faxphone B640"},
 {"\x80\x00\x80\x49\x10", "Fax B100"},
 {"\x80\x00\x8A\x49\x10", "Laser Class 9000 Series"},
 {"\x80\x00\x8A\x48\x00", "Laser Class 2060"},
 {NULL}};
  

static const ModelData Brother[] =
{{"\x55\x55\x00\x88\x90\x80\x5F\x00\x15\x51", "Fax-560/770"},
 {"\x55\x55\x00\x80\xB0\x80\x00\x00\x59\xD4", "Personal fax 190"},
 {"\x55\x55\x00\x8C\x90\x80", "MFC-3100C/MFC-8600"},
 {NULL}};

static const ModelData Panasonic0E[] =
{{"\x00\x00\x00\x96\x0F\x01\x02\x00\x10\x05\x02\x95\xC8\x08\x01\x49\x02\x41\x53\x54\x47", "KX-F90" },
 {"\x00\x00\x00\x96\x0F\x01\x03\x00\x10\x05\x02\x95\xC8\x08\x01\x49\x02\x03", "KX-F230/KX-FT21" },
 {"\x00\x00\x00\x16\x0F\x01\x03\x00\x10\x05\x02\x95\xC8\x08",          "KX-F780" },
 {"\x00\x00\x00\x16\x0F\x01\x03\x00\x10\x00\x02\x95\x80\x08\x75\xB5",  "KX-M260" },
 {"\x00\x00\x00\x16\x0F\x01\x02\x00\x10\x05\x02\x85\xC8\x08\xAD", "KX-F2050BS" },
 {NULL}};

static const ModelData Panasonic79[] =
{{"\x00\x00\x00\x02\x0F\x09\x12\x00\x10\x05\x02\x95\xC8\x88\x80\x80\x01", "UF-S10" },
 {"\x00\x00\x00\x16\x7F\x09\x13\x00\x10\x05\x16\x8D\xC0\xD0\xF8\x80\x01", "/Siemens Fax 940" },
 {"\x00\x00\x00\x16\x0F\x09\x13\x00\x10\x05\x06\x8D\xC0\x50\xCB", "Panafax UF-321" },
 {NULL}};

static const ModelData Ricoh[] =
{{"\x00\x00\x00\x12\x10\x0D\x02\x00\x50\x00\x2A\xB8\x2C", "/Nashuatec P394" },
 {NULL}};

static const ModelData Samsung16[] =
{{"\x00\x00\xA4\x01", "M545 6800" },
 {NULL}};

static const ModelData Samsung8C[] =
{{"\x00\x00\x01\x00", "SF-2010" },
 {NULL}};

static const ModelData SamsungA2[] =
{{"\x00\x00\x80\x00", "FX-4000" },
 {NULL}};

static const ModelData Sanyo[] =
{{"\x00\x00\x10\xB1\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x41\x26\xFF\xFF\x00\x00\x85\xA1", "SFX-107" },
 {"\x00\x00\x00\xB1\x12\xF2\x62\xB4\x82\x0A\xF2\x2A\x12\xD2\xA2\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x41\x4E\xFF\xFF\x00\x00", "MFP-510" },
 {NULL}};

static const ModelData HP[] =
{{"\x20\x00\x45\x00\x0C\x04\x70\xCD\x4F\x00\x7F\x49", "LaserJet 3150" },
 {"\x04\x00\x00\x00\x00", "LaserJet 3030" },	/* gert */
 {"\x40\x80\x84\x01\xF0\x6A", "OfficeJet" },
 {"\xC0\x00\x00\x00\x00", "OfficeJet 500" },
 {"\xC0\x00\x00\x00\x00\x8B", "Fax-920" },
 {NULL}};

static const ModelData Sharp[] =
{{"\x00\xCE\xB8\x80\x80\x11\x85\x0D\xDD\x00\x00\xDD\xDD\x00\x00\xDD\xDD\x00\x00\x00\x00\x00\x00\x00\x00\xED\x22\xB0\x00\x00\x90\x00\x8C", "Sharp UX-460" },
 {"\x00\x4E\xB8\x80\x80\x11\x84\x0D\xDD\x00\x00\xDD\xDD\x00\x00\xDD\xDD\x00\x00\x00\x00\x00\x00\x00\x00\xED\x22\xB0\x00\x00\x90\x00\xAD", "Sharp UX-177" },
 {"\x00\xCE\xB8\x00\x84\x0D\xDD\x00\x00\xDD\xDD\x00\x00\xDD\xDD\xDD\xDD\xDD\x02\x05\x28\x02\x22\x43\x29\xED\x23\x90\x00\x00\x90\x01\x00", "Sharp FO-4810" },
 {NULL}};

static const ModelData Xerox[] =
{{"\x00\x08\x2D\x43\x57\x50\x61\x75\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x1A\x02\x02\x10\x01\x82\x01\x30\x34", "635 Workcenter" },
 {"\x00\x08\x0D\x49\xFF\xFF\xFF\xFF\xFF\xFF", "DP85F" },	/* gert */
 {NULL}};

static const ModelData XeroxDA[] =
{{"\x00\x00\xC0\x00", "Workcentre Pro 580" },
 {NULL}};

static const ModelData Lexmark[] =
{{"\x00\x80\xA0\x00", "X4270" },
 {NULL}};

static const ModelData JetFax[] =
{{"\x01\x00\x45\x00\x0D\x7F", "M910e" },
 {NULL}};

static const ModelData PitneyBowes[] = 
{{"\x79\x91\xB1\xB8\x7A\xD8", "9550" },
 {NULL}};

static const ModelData Muratec45[] =
{{"\xF4\x91\xFF\xFF\xFF\x42\x2A\xBC\x01\x57", "M4700" },
 {NULL}};

static const ModelData Muratec48[] =
{{"\x53\x53\x61", "M620" },
 {NULL}};

/*
 * Country code first byte, then manufacturer is last two bytes. See T.35.
 * Apparently Germany issued some manufacturer codes before the two-byte
 * standard was accepted, and so some few German manufacturers are
 * identified by a single manufacturer byte.
 *
 * T.30 5.3.6.2.7 (2003) states that the NSF FIF is transmitted
 * in MSB2LSB order.  Revisions of T.30 prior to 2003 did not 
 * contain explicit specification as to the transmit bit order.
 * (Although it did otherwise state that all HDLC frame data should
 * be in MSB order except as noted.)  Because CSI, TSI, and other
 * prologue frames were in LSB order by way of an exception to the
 * general rule (T.30 5.3.6.2.4-11) many manufacturers assumed that
 * NSF should also be in LSB order.  Consequently there will be
 * some country-code "masquerading" as a terminal may use the
 * proper country-code, but with an inverted bit order.
 *
 * Thus, country code x61 (Korea) turns into x86 (Papua New Guinea),
 * code xB5 (USA) turns into xAD (Tunisia), code x26 (China) turns
 * into x64 (Lebanon), code x04 (Germany) turns into x20 (Canada), 
 * and code x3D (France) turns into xBC (Vietnam).
 *
 * For the most part it should be safe to identify a manufacturer
 * both with the MSB and LSB ordered bits, as the "masqueraded" country
 * is likely to not be actively assigning T.35 manufacturer codes.
 * However, some manufacturers (e.g. Microsoft) may use MSB for the
 * country code and LSB for the rest of the NSF, and so basically this
 * table must be verified and corrected against actual real-world
 * results.
 */
static const NSFData KnownNSF[] =
{
    /* Japan */
    {"\x00\x00\x00", 3, "unknown - indeterminate", true },
    {"\x00\x00\x01", 3, "Anjitsu",  false },
    {"\x00\x00\x02", 3, "Nippon Telephone", false },
    {"\x00\x00\x05", 3, "Mitsuba Electric", false },
    {"\x00\x00\x06", 3, "Master Net", false },
    {"\x00\x00\x09", 3, "Xerox/Toshiba", true, 3, 10, Xerox },
    {"\x00\x00\x0A", 3, "Kokusai",   false },
    {"\x00\x00\x0D", 3, "Logic System International", false },
    {"\x00\x00\x0E", 3, "Panasonic", false, 3,10, Panasonic0E },
    {"\x00\x00\x11", 3, "Canon",     false, 3, 5, Canon },
    {"\x00\x00\x15", 3, "Toyotsushen Machinery", false },
    {"\x00\x00\x16", 3, "System House Mind", false },
    {"\x00\x00\x19", 3, "Xerox",     true  },
    {"\x00\x00\x1D", 3, "Hitachi Software", false },
    {"\x00\x00\x21", 3, "Oki Electric/Lanier", true },
    {"\x00\x00\x25", 3, "Ricoh",     true,  3,10, Ricoh },
    {"\x00\x00\x26", 3, "Konica",    false },
    {"\x00\x00\x29", 3, "Japan Wireless", false },
    {"\x00\x00\x2D", 3, "Sony",      false },
    {"\x00\x00\x31", 3, "Sharp/Olivetti", false, 3, 10, Sharp },
    {"\x00\x00\x35", 3, "Kogyu", false },
    {"\x00\x00\x36", 3, "Japan Telecom", false },
    {"\x00\x00\x3D", 3, "IBM Japan", false },
    {"\x00\x00\x39", 3, "Panasonic", false },
    {"\x00\x00\x41", 3, "Swasaki Communication", false },
    {"\x00\x00\x45", 3, "Muratec",   false, 3,10, Muratec45 },
    {"\x00\x00\x46", 3, "Pheonix",   false },
    {"\x00\x00\x48", 3, "Muratec",   false, 3,3, Muratec48 },	/* not registered */
    {"\x00\x00\x49", 3, "Japan Electric", false },
    {"\x00\x00\x4D", 3, "Okura Electric", false },
    {"\x00\x00\x51", 3, "Sanyo",     false, 3,10, Sanyo },
    {"\x00\x00\x55", 3, "unknown - Japan 55", false },
    {"\x00\x00\x56", 3, "Brother",   false, 3, 6, Brother },
    {"\x00\x00\x59", 3, "Fujitsu",   false },
    {"\x00\x00\x5D", 3, "Kuoni",     false },
    {"\x00\x00\x61", 3, "Casio",     false },
    {"\x00\x00\x65", 3, "Tateishi Electric", false },
    {"\x00\x00\x66", 3, "Utax/Mita", true  },
    {"\x00\x00\x69", 3, "Hitachi Production",   false },
    {"\x00\x00\x6D", 3, "Hitachi Telecom", false },
    {"\x00\x00\x71", 3, "Tamura Electric Works", false },
    {"\x00\x00\x75", 3, "Tokyo Electric Corp.", false },
    {"\x00\x00\x76", 3, "Advance",   false },
    {"\x00\x00\x79", 3, "Panasonic", false, 3,10, Panasonic79 },
    {"\x00\x00\x7D", 3, "Seiko",     false },
    {"\x00\x08\x00", 3, "Daiko",     false },
    {"\x00\x10\x00", 3, "Funai Electric", false },
    {"\x00\x20\x00", 3, "Eagle System", false },
    {"\x00\x30\x00", 3, "Nippon Business Systems", false },
    {"\x00\x40\x00", 3, "Comtron",   false },
    {"\x00\x48\x00", 3, "Cosmo Consulting", false },
    {"\x00\x50\x00", 3, "Orion Electric", false },
    {"\x00\x60\x00", 3, "Nagano Nippon", false },
    {"\x00\x70\x00", 3, "Kyocera",   false },
    {"\x00\x80\x00", 3, "Kanda Networks", false },
    {"\x00\x88\x00", 3, "Soft Front", false },
    {"\x00\x90\x00", 3, "Arctic",    false },
    {"\x00\xA0\x00", 3, "Nakushima", false },
    {"\x00\xB0\x00", 3, "Minolta", false },
    {"\x00\xC0\x00", 3, "Tohoku Pioneer", false },
    {"\x00\xD0\x00", 3, "USC",       false },
    {"\x00\xE0\x00", 3, "Hiboshi",   false },
    {"\x00\xF0\x00", 3, "Sumitomo Electric", false },
    /* Germany */
    {"\x20\x09",     2, "ITK Institut fuer Telekommunikation GmbH & Co KG", false },
    {"\x20\x11",     2, "Dr. Neuhaus Mikroelektronik", false },
    {"\x20\x21",     2, "ITO Communication", false },
    {"\x20\x31",     2, "mbp Kommunikationssysteme GmbH", false },
    {"\x20\x41",     2, "Siemens", false },
    {"\x20\x42",     2, "Deutsche Telekom AG", false },
    {"\x20\x51",     2, "mps Software", false },
    {"\x20\x61",     2, "Hauni Elektronik", false },
    {"\x20\x71",     2, "Digitronic computersysteme gmbh", false },
    {"\x20\x81\x00", 3, "Innovaphone GmbH", false },
    {"\x20\x81\x40", 3, "TEDAS Gesellschaft fuer Telekommunikations-, Daten- und Audiosysteme mbH", false },
    {"\x20\x81\x80", 3, "AVM Audiovisuelles Marketing und Computersysteme GmbH", false },
    {"\x20\x81\xC0", 3, "EICON Technology Research GmbH", false },
    {"\x20\x81\x70", 3, "mgetty+sendfax", false },
    {"\x20\xB1",     2, "Schneider Rundfunkwerke AG", false },
    {"\x20\xC2",     2, "Deutsche Telekom AG", false },
    {"\x20\xD1",     2, "Ferrari electronik GmbH", false },
    {"\x20\xF1",     2, "DeTeWe - Deutsche Telephonwerke AG & Co", false },
    {"\x20\xFF",     2, "Germany Regional Code", false },
    /* China */
    {"\x64\x00\x00", 3, "unknown - China 00 00", false },
    {"\x64\x01\x00", 3, "unknown - China 01 00", false },
    {"\x64\x01\x01", 3, "unknown - China 01 01", false },
    {"\x64\x01\x02", 3, "unknown - China 01 02", false },
    /* France */
    {"\xBC\x53\x01", 3, "Minolta",   false },
    /* Korea */
    {"\x61\x00\x7A", 3, "Xerox", false },
    {"\x86\x00\x02", 3, "unknown - Korea 02", false },
    {"\x86\x00\x06", 3, "unknown - Korea 06", false },
    {"\x86\x00\x08", 3, "unknown - Korea 08", false },
    {"\x86\x00\x0A", 3, "unknown - Korea 0A", false },
    {"\x86\x00\x0E", 3, "unknown - Korea 0E", false },
    {"\x86\x00\x10", 3, "Samsung",   false },
    {"\x86\x00\x11", 3, "unknown - Korea 11" },
    {"\x86\x00\x16", 3, "Samsung", false, 3, 4, Samsung16 },
    {"\x86\x00\x1A", 3, "unknown - Korea 1A", false },
    {"\x86\x00\x40", 3, "unknown - Korea 40", false },
    {"\x86\x00\x48", 3, "Samsung/Dell", false },
    {"\x86\x00\x52", 3, "unknown - Korea 52", false },
    {"\x86\x00\x5A", 3, "Samsung", false },
    {"\x86\x00\x5E", 3, "Xerox", false },
    {"\x86\x00\x66", 3, "unknown - Korea 66", false },
    {"\x86\x00\x6E", 3, "unknown - Korea 6E", false },
    {"\x86\x00\x82", 3, "unknown - Korea 82", false },
    {"\x86\x00\x88", 3, "Ricoh", false },
    {"\x86\x00\x8A", 3, "unknown - Korea 8A", false },
    {"\x86\x00\x8C", 3, "Samsung", false, 3, 4, Samsung8C },
    {"\x86\x00\x92", 3, "unknown - Korea 92", false },
    {"\x86\x00\x98", 3, "Samsung",   false },
    {"\x86\x00\xA2", 3, "Samsung", false, 3, 4, SamsungA2 },
    {"\x86\x00\xA4", 3, "unknown - Korea A4", false },
    {"\x86\x00\xC2", 3, "Samsung", false },
    {"\x86\x00\xC9", 3, "unknown - Korea C9", false },
    {"\x86\x00\xCC", 3, "unknown - Korea CC", false },
    {"\x86\x00\xD2", 3, "unknown - Korea D2", false },
    {"\x86\x00\xDA", 3, "Xerox", false, 3, 4, XeroxDA },
    {"\x86\x00\xE2", 3, "unknown - Korea E2", false },
    {"\x86\x00\xEC", 3, "unknown - Korea EC", false },
    {"\x86\x00\xEE", 3, "unknown - Korea EE", false },
    /* United Kingdom */
    {"\xB4\x00\xB0", 3, "DCE",       false },
    {"\xB4\x00\xB1", 3, "Hasler",    false },
    {"\xB4\x00\xB2", 3, "Interquad", false },
    {"\xB4\x00\xB3", 3, "Comwave",   false },
    {"\xB4\x00\xB4", 3, "Iconographic", false },
    {"\xB4\x00\xB5", 3, "Wordcraft", false },
    {"\xB4\x00\xB6", 3, "Acorn",     false },
    /* United States */
    {"\xAD\x00\x00", 3, "Pitney Bowes", false, 3, 6, PitneyBowes },
    {"\xAD\x00\x0C", 3, "Dialogic",  false },
    {"\xAD\x00\x15", 3, "Lexmark",   false, 3, 4, Lexmark },
    {"\xAD\x00\x16", 3, "JetFax",    false, 3, 6, JetFax },
    {"\xAD\x00\x24", 3, "Octel",     false },
    {"\xAD\x00\x36", 3, "HP",        false, 3, 5, HP },
    {"\xAD\x00\x42", 3, "FaxTalk",   false },
    {"\xAD\x00\x44", 3, NULL,        true },
    {"\xAD\x00\x46", 3, "BrookTrout", false },
    {"\xAD\x00\x51", 3, "Telogy Networks", false },
    {"\xAD\x00\x55", 3, "HylaFAX",   false },
    {"\xAD\x00\x5C", 3, "IBM", false },
    {"\xAD\x00\x98", 3, "unknown - USA 98", true },
    {"\xB5\x00\x01", 3, "Picturetel", false },
    {"\xB5\x00\x20", 3, "Conexant",  false },
    {"\xB5\x00\x22", 3, "Comsat",    false },
    {"\xB5\x00\x24", 3, "Octel",     false },
    {"\xB5\x00\x26", 3, "ROLM",      false },
    {"\xB5\x00\x28", 3, "SOFNET",    false },
    {"\xB5\x00\x29", 3, "TIA TR-29 Committee", false },
    {"\xB5\x00\x2A", 3, "STF Tech",  false },
    {"\xB5\x00\x2C", 3, "HKB",       false },
    {"\xB5\x00\x2E", 3, "Delrina",   false },
    {"\xB5\x00\x30", 3, "Dialogic",  false },
    {"\xB5\x00\x32", 3, "Applied Synergy", false },
    {"\xB5\x00\x34", 3, "Syncro Development", false },
    {"\xB5\x00\x36", 3, "Genoa",     false },
    {"\xB5\x00\x38", 3, "Texas Instruments", false },
    {"\xB5\x00\x3A", 3, "IBM",       false },
    {"\xB5\x00\x3C", 3, "ViaSat",    false },
    {"\xB5\x00\x3E", 3, "Ericsson",  false },
    {"\xB5\x00\x42", 3, "Bogosian",  false },
    {"\xB5\x00\x44", 3, "Adobe",     false },
    {"\xB5\x00\x46", 3, "Fremont Communications", false },
    {"\xB5\x00\x48", 3, "Hayes",     false },
    {"\xB5\x00\x4A", 3, "Lucent",    false },
    {"\xB5\x00\x4C", 3, "Data Race", false },
    {"\xB5\x00\x4E", 3, "TRW",       false },
    {"\xB5\x00\x52", 3, "Audiofax",  false },
    {"\xB5\x00\x54", 3, "Computer Automation", false },
    {"\xB5\x00\x56", 3, "Serca",     false },
    {"\xB5\x00\x58", 3, "Octocom",   false },
    {"\xB5\x00\x5C", 3, "Power Solutions", false },
    {"\xB5\x00\x5A", 3, "Digital Sound", false },
    {"\xB5\x00\x5E", 3, "Pacific Data", false },
    {"\xB5\x00\x60", 3, "Commetrex", false },
    {"\xB5\x00\x62", 3, "BrookTrout", false },
    {"\xB5\x00\x64", 3, "Gammalink", false },
    {"\xB5\x00\x66", 3, "Castelle",  false },
    {"\xB5\x00\x68", 3, "Hybrid Fax", false },
    {"\xB5\x00\x6A", 3, "Omnifax",   false },
    {"\xB5\x00\x6C", 3, "HP",        false },
    {"\xB5\x00\x6E", 3, "Microsoft", false },
    {"\xB5\x00\x72", 3, "Speaking Devices", false },
    {"\xB5\x00\x74", 3, "Compaq",    false },
/*
    {"\xB5\x00\x76", 3, "Trust - Cryptek", false },	// collision with Microsoft
*/
    {"\xB5\x00\x76", 3, "Microsoft", false },		/* uses LSB for country but MSB for manufacturer */
    {"\xB5\x00\x78", 3, "Cylink",    false },
    {"\xB5\x00\x7A", 3, "Pitney Bowes", false },
    {"\xB5\x00\x7C", 3, "Digiboard", false },
    {"\xB5\x00\x7E", 3, "Codex",     false },
    {"\xB5\x00\x82", 3, "Wang Labs", false },
    {"\xB5\x00\x84", 3, "Netexpress Communications", false },
    {"\xB5\x00\x86", 3, "Cable-Sat", false },
    {"\xB5\x00\x88", 3, "MFPA",      false },
    {"\xB5\x00\x8A", 3, "Telogy Networks", false },
    {"\xB5\x00\x8E", 3, "Telecom Multimedia Systems", false },
    {"\xB5\x00\x8C", 3, "AT&T",      false },
    {"\xB5\x00\x92", 3, "Nuera",     false },
    {"\xB5\x00\x94", 3, "K56flex",   false },
    {"\xB5\x00\x96", 3, "MiBridge",  false },
    {"\xB5\x00\x98", 3, "Xerox",     false },
    {"\xB5\x00\x9A", 3, "Fujitsu",   false },
    {"\xB5\x00\x9B", 3, "Fujitsu",   false },
    {"\xB5\x00\x9C", 3, "Natural Microsystems",  false },
    {"\xB5\x00\x9E", 3, "CopyTele",  false },
    {"\xB5\x00\xA2", 3, "Murata",    false },
    {"\xB5\x00\xA4", 3, "Lanier",    false },
    {"\xB5\x00\xA6", 3, "Qualcomm",  false },
    {"\xB5\x00\xAA", 3, "HylaFAX",   false },		/* they did it backwards for a while */
    {NULL}
};

void hylafax_nsf_decode _P2(( nsf, nsfSize ), 
			     unsigned char * nsf, int nsfSize )
{
const char * vendor = NULL;
const char * model = NULL;
const NSFData * p;
const ModelData * pp;

    for( p = KnownNSF; p->vendorId; p++ ){
        if( nsfSize >= p->vendorIdSize &&
            memcmp( p->vendorId, &nsf[0], p->vendorIdSize )==0 ){
	    if( p->vendorName )
                vendor = p->vendorName;
            if( p->knownModels ){
                for( pp = p->knownModels; pp->modelId; pp++ )
                    if( nsfSize >= p->modelIdPos + p->modelIdSize &&
                        memcmp( pp->modelId, &nsf[p->modelIdPos], p->modelIdSize )==0 )
                        model = pp->modelName;
            }
	    break;
        }
    }
    if ( vendor != NULL || model != NULL )
	lprintf( L_MESG, "NSF: vendor=%s, model=%s",
			vendor != NULL? vendor: "<unknown>",
			model  != NULL? model : "<unknown>" );
}

#endif
