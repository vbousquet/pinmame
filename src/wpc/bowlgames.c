// license:BSD-3-Clause

/*
 * In this file we'll collect any bowler, shuffle alley, pitch & bat game,
 * and other non-pinball stuff that's running on pinball machine hardware.
 */

#include "driver.h"
#include "core.h"
#include "by35.h"
#include "by35snd.h"
#include "sndbrd.h"
#include "wmssnd.h"
#include "wpc.h"
#include "s11.h"
#include "s4.h"
#include "gts3.h"
#include "gts80s.h"
#include "dedmd.h"
#include "desound.h"
#include "se.h"
#include "inder.h"
#include "vpintf.h"
#include "machine/6821pia.h"

#define INITGAME(name, gen, disp) \
static core_tGameData name##GameData = { gen, disp }; \
static void init_##name(void) { core_gameData = &name##GameData; }

#define INITGAME_S10(name, gen, disp, flags, flip, db, inv) \
static core_tGameData name##GameData = { \
  gen, disp, {flip,0,0,0,0,db,flags}, NULL, {{0}, {0,0,0,0,inv}}}; \
static void init_##name(void) { core_gameData = &name##GameData; }

S4_INPUT_PORTS_START(bowl, 1) S4_INPUT_PORTS_END

static const core_tLCDLayout dispS5[] = {
  { 0, 0, 0, 4, CORE_SEG7 }, { 0,12, 4, 4, CORE_SEG7 },
  { 2, 0, 8, 4, CORE_SEG7 }, { 2,12,20, 4, CORE_SEG7 },
  { 4, 0,24, 4, CORE_SEG7 }, { 4,12,28, 4, CORE_SEG7 },
  { 6, 4,12, 2, CORE_SEG7 }, { 6,12,14, 2, CORE_SEG7 }, {0}
};

static const core_tLCDLayout dispS10[] = {
  { 0, 0,22, 4, CORE_SEG7 }, { 0,12,30, 4, CORE_SEG7 },
  { 2, 0, 2, 4, CORE_SEG7 }, { 2,12,10, 4, CORE_SEG7 },
  { 4, 0,26, 2, CORE_SEG7 }, { 4, 4,34, 2, CORE_SEG7 }, { 4,12, 6, 2, CORE_SEG7 }, { 4,16,14, 2, CORE_SEG7 },
  { 6, 4, 0, 1, CORE_SEG7 }, { 6, 6, 8, 1, CORE_SEG7 }, { 6,12,20, 1, CORE_SEG7 }, { 6,14,28, 1, CORE_SEG7 }, {0}
};

static const core_tLCDLayout dispBowl[] = {
  { 0, 0, 4, 4, CORE_SEG7 }, { 0,12,12, 4, CORE_SEG7 },
  { 2, 0,20, 4, CORE_SEG7 }, { 2,12,28, 4, CORE_SEG7 },
  { 4, 0,36, 4, CORE_SEG7 }, { 4,12,44, 4, CORE_SEG7 },
  { 6, 4,54, 2, CORE_SEG7 }, { 6,12,52, 2, CORE_SEG7 }, {0}
};

static struct core_dispLayout de_dmd192x64[] = {
  {0,0,64,192,CORE_DMD,(genf *)dedmd64_update}, {0}
};

static struct core_dispLayout se_dmd128x32[] = {
  {0,0, 32,128, CORE_DMD, (genf *)dedmd32_update}, {0}
};

/*----------------------------
/ Pompeii
/----------------------------*/
INITGAME(pomp, GEN_S3, dispS5)
S4_ROMSTART(pomp,l1,"gamerom.716",CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee),
                     "b_ic20.716", CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e),
                     "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S67S_SOUNDROMS8("soundx.716", CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
S4_ROMEND
#define input_ports_pomp input_ports_bowl
CORE_GAMEDEF(pomp,l1,"Pompeii (Shuffle) (L-1)",1978,"Williams",s4_mS4S,0)

/*----------------------------
/ Aristocrat
/----------------------------*/
INITGAME(arist, GEN_S3, dispS5)
S4_ROMSTART(arist,l1,"gamerom.716",CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee),
                     "b_ic20.716", CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e),
                     "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S67S_SOUNDROMS8("soundx.716", CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
S4_ROMEND
#define input_ports_arist input_ports_bowl
CORE_GAMEDEF(arist,l1,"Aristocrat (Shuffle) (L-1)",1979,"Williams",s4_mS4S,0)

/*----------------------------
/ Topaz
/----------------------------*/
INITGAME(topaz, GEN_S3, dispS5)
S4_ROMSTART(topaz,l1,"gamerom.716",CRC(cb287b10) SHA1(7fb6b6a26237cf85d5e02cf35271231267f90fc1),
                     "b_ic20.716", CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e),
                     "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S67S_SOUNDROMS8("soundx.716", CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
//S67S_SOUNDROMS8("sound1.716",CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
S4_ROMEND
#define input_ports_topaz input_ports_bowl
CORE_GAMEDEF(topaz,l1,"Topaz (Shuffle) (L-1)",1978,"Williams",s4_mS4S,0)

/*----------------------------
/ Taurus
/----------------------------*/
INITGAME(taurs, GEN_S4, dispS5)
S4_ROMSTART(taurs,l1,"gamerom.716",CRC(3246e285) SHA1(4f76784ecb5063a49c24795ae61db043a51e2c89),
                     "b_ic20.716", CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e),
                     "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S67S_SOUNDROMS8("soundx.716", CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
//S67S_SOUNDROMS8("sound1.716", CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
S4_ROMEND
#define input_ports_taurs input_ports_bowl
CORE_GAMEDEF(taurs,l1,"Taurus (Shuffle) (L-1)",1979,"Williams",s4_mS4S,0)

/*----------------------------
/ King Tut
/----------------------------*/
INITGAME(kingt, GEN_S4, dispS5)
S4_ROMSTART(kingt,l1,"gamerom.716",CRC(54d3280a) SHA1(ca74636e35d2c3e0b3133f89b1ff1233d5d72a5c),
                     "b_ic20.716", CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e),
                     "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S67S_SOUNDROMS8("soundx.716", CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
S4_ROMEND
#define input_ports_kingt input_ports_bowl
CORE_GAMEDEF(kingt,l1,"King Tut (Shuffle) (L-1)",1979,"Williams",s4_mS4S,0)

/*----------------------------
/ Omni
/----------------------------*/
INITGAME(omni, GEN_S4, dispS5)
S4_ROMSTART(omni,l1,"omni-1a.u21",CRC(443bd170) SHA1(cc1ebd72d77ec2014cbd84534380e5ea1f12c022),
                    "b_ic20.716", CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e),
                    "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S67S_SOUNDROMS8("sound.716", CRC(db085cbb) SHA1(9a57abbad183ba16b3dba16d16923c3bfc46a0c3))
S4_ROMEND
#define input_ports_omni input_ports_bowl
CORE_GAMEDEF(omni,l1,"Omni (Shuffle) (L-1)",1980,"Williams",s4_mS4S,0)

/*--------------------------------
/ Big Ball Bowling (United game?)
/-------------------------------*/
static core_tGameData bbbowlinGameData = {GEN_BOWLING,dispBowl,{FLIP_SW(FLIP_L),0,0,0,SNDBRD_ST100B,0}};
static void init_bbbowlin(void) { core_gameData = &bbbowlinGameData; }
BY17_ROMSTARTx88(bbbowlin,"cpu_u2.716",CRC(179e0c69) SHA1(7921839d2014a00b99ce7c44b325ea4403df9eea),
                          "cpu_u6.716",CRC(7b48e45b) SHA1(ac32292ef593bf8350e8bbc41113b6c1cb78a79e))
BY35_ROMEND
BY35_INPUT_PORTS_START(bbbowlin,1) BY35_INPUT_PORTS_END
CORE_GAMEDEFNV(bbbowlin,"Big Ball Bowling (Bowler)",19??,"United(?)",by35_mBowling,0)

/*----------------------------
/ Stars & Strikes
/----------------------------*/
static core_tGameData monrobwlGameData = {GEN_BOWLING,dispBowl,{FLIP_SW(FLIP_L),0,0,0,SNDBRD_ST100B,0}};
static void init_monrobwl(void) { core_gameData = &monrobwlGameData; }
ST200_ROMSTART8888(monrobwl,"cpu_u1.716",CRC(42592cc9) SHA1(22452072199c4b82a413065f8dfe235a39fe3825),
                            "cpu_u5.716",CRC(78e2dcd2) SHA1(7fbe9f7adc69af5afa489d9fd953640f3466de3f),
                            "cpu_u2.716",CRC(73534680) SHA1(d5233a9d4600fa28b767ee1a251ed1a1ffbaf9c4),
                            "cpu_u6.716",CRC(ad77d719) SHA1(f8f8d0d183d639d19fea552d35a7be3aa7f07c17))
BY35_ROMEND
BY35_INPUT_PORTS_START(monrobwl, 1) BY35_INPUT_PORTS_END
CORE_GAMEDEFNV(monrobwl,"Stars & Strikes (Bowler)",19??,"Monroe Bowling Co.",by35_mBowling2,0)

/*----------------------------
/ Big Strike
/----------------------------*/
INITGAME(bstrk, GEN_S4, dispS5)
S4_ROMSTART(bstrk,l1,"gamerom.716",CRC(323dbcde) SHA1(a75cbb5de97cb9afc1d36e9b6ff593bb482fcf8b),
                     "b_ic20.716", CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e),
                     "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S4_ROMEND
#define input_ports_bstrk input_ports_bowl
CORE_GAMEDEF(bstrk,l1,"Big Strike (Shuffle) (L-1)",1983,"Williams",s4_mS4,GAME_USES_CHIMES)

/*----------------------------
/ Triple Strike
/----------------------------*/
INITGAME(tstrk, GEN_S4, dispS5)
S4_ROMSTART(tstrk,l1,"gamerom.716",CRC(b034c059) SHA1(76b3926b87b3c137fcaf33021a586827e3c030af),
                     "ic20.716",   CRC(f163fc88) SHA1(988b60626f3d4dc8f4a1dbd0c99282418bc53aae),
                     "b_ic17.716", CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
S4_ROMEND
#define input_ports_tstrk input_ports_bowl
CORE_GAMEDEF(tstrk,l1,"Triple Strike (Shuffle) (L-1)",1983,"Williams",s4_mS4,GAME_USES_CHIMES)

/*--------------------------------
/ Big Bat
/-------------------------------*/
static core_tLCDLayout dispBigbat[] = {
  {0, 0,34, 6,CORE_SEG7}, {0}
};
static core_tGameData bigbatGameData = {GEN_BY35,dispBigbat,{FLIP_SW(FLIP_L),0,0,0,SNDBRD_BY61,0,BY35GD_NOSOUNDE}};
static void init_bigbat(void) { core_gameData = &bigbatGameData; }
BY35_ROMSTARTx00(bigbat,"u2.bin",CRC(2beda24d) SHA1(80fb9ed548e4886741c709979aa4f865f47d2257),
                          "u6.bin",CRC(8f13469d) SHA1(00c626f7eb166f627f6498d75906b3c56bccdd62))
BY61_SOUNDROMx000(        "u3.bin",CRC(b87a9335) SHA1(8a21bcbcbe91da1bab0af06b71604bb8f247d0d4),
                          "u4.bin",CRC(4ab75b31) SHA1(46acd1c9250a635b51bffccd77ea4e67a0c5edf5),
                          "u5.bin",CRC(0aec8204) SHA1(f44216cccc3652399549345d8c74bcae54662aa3))
BY35_ROMEND
BY35_INPUT_PORTS_START(bigbat,1) BY35_INPUT_PORTS_END
CORE_GAMEDEFNV(bigbat,"Big Bat",1984,"Bally Midway",by35_mBY35_61S,0)

/*------------------------------------------------
/ Midnight Marauders (BY35-???: 05/84) - Gun game
/-------------------------------------------------*/
static SWITCH_UPDATE(marauders) {
  if (inports) {
    UINT8 col1 = inports[BY35_COMINPORT] & 0x60;
    UINT8 col2 = (inports[BY35_COMINPORT] >> 8) & 0x87;
    if (col1 & 0x20) col1 = (col1 & 0xdf) | 0x01; // move start sw
    col1 |= (col1 & 0x40) >> 1; // move tilt sw
    col2 |= (col2 & 0x80) >> 4; // move slam sw
    CORE_SETKEYSW(inports[BY35_COMINPORT], 0x07, 0);
    CORE_SETKEYSW(col1, 0x21, 1);
    CORE_SETKEYSW(col2, 0x0f, 2);
  }
  /*-- Diagnostic buttons on CPU board --*/
  cpu_set_nmi_line(0, core_getSw(BY35_SWCPUDIAG) ? ASSERT_LINE : CLEAR_LINE);
  sndbrd_0_diag(core_getSw(BY35_SWSOUNDDIAG));
  /*-- coin door switches --*/
  pia_set_input_ca1(0, !core_getSw(BY35_SWSELFTEST));
}
static MACHINE_DRIVER_START(marauders)
  MDRV_IMPORT_FROM(by35_61S)
  MDRV_SWITCH_UPDATE(marauders)
MACHINE_DRIVER_END
static const core_tLCDLayout dispMM[] = {
  {0, 0, 7,1,CORE_SEG7}, {0, 2, 6,1,CORE_SEG7}, {0, 4, 5,1,CORE_SEG7},
  {0, 6, 4,1,CORE_SEG7}, {0, 8, 3,1,CORE_SEG7}, {0,10, 2,1,CORE_SEG7}, {0}
};
static core_tGameData mdntmrdrGameData = {GEN_BY35,dispMM,{FLIP_SW(FLIP_L),0,8,0,SNDBRD_BY61,0,BY35GD_MARAUDER}};
static void init_mdntmrdr(void) { core_gameData = &mdntmrdrGameData; }
BY35_ROMSTARTx00(mdntmrdr, "mdru2.532", CRC(f72668bc) SHA1(25b984e1828905190c73c359ee6c9858ed1b2224),
                           "mdru6.732", CRC(ff55fb57) SHA1(4a44fc8732c8cbce38c9605c7958b02a6bc95da1))
SOUNDREGION(0x10000, REGION_CPU2)
  ROM_LOAD("u3.bin", 0xd000, 0x1000, CRC(3ba474e4) SHA1(4ee5c3ad2c9dca49e9394521506e97a95e3d9a17))
  ROM_LOAD("u5.bin", 0xf000, 0x1000, CRC(3ab40e35) SHA1(63b2ee074e5993a2616e67d3383bc3d3ac51b400))
BY35_ROMEND
BY35_INPUT_PORTS_START(mdntmrdr,1) BY35_INPUT_PORTS_END
CORE_GAMEDEFNV(mdntmrdr,"Midnight Marauders (Gun game)",1984,"Bally Midway",marauders,0)

/*----------------------------
/ Black Beauty
/----------------------------*/
static core_tGameData blbeautyGameData = {GEN_BOWLING,dispBowl,{FLIP_SW(FLIP_L),0,0,0,SNDBRD_ST100B,0}};
static void init_blbeauty(void) { core_gameData = &blbeautyGameData; }
ST200_ROMSTART8888(blbeauty,"cpu_u1.716",CRC(e2550957) SHA1(e445548b650fec5d593ca7da587300799ef94991),
                            "cpu_u5.716",CRC(70fcd9f7) SHA1(ca5c2ea09f45f5ba50526880c158aaac61f007d5),
                            "cpu_u2.716",CRC(3f55d17f) SHA1(e6333e53570fb05a841a7f141872c8bd14143f9c),
                            "cpu_u6.716",CRC(842cd307) SHA1(8429d84e8bc4343b437801d0236150e04de79b75))
BY35_ROMEND
BY35_INPUT_PORTS_START(blbeauty, 1) BY35_INPUT_PORTS_END
CORE_GAMEDEFNV(blbeauty,"Black Beauty (Shuffle)",1984,"Stern",by35_mBowling2,0)

/*--------------------
/ Pennant Fever (#526)
/--------------------*/
static core_tLCDLayout dispPfevr[] = {
  { 0, 0,21, 3, CORE_SEG7 }, { 0, 8,25, 3, CORE_SEG7 }, { 0,16,12, 4, CORE_SEG7 },
  { 2, 2, 0, 1, CORE_SEG7 }, { 2, 4, 8, 1, CORE_SEG7 },
  { 2, 8,20, 1, CORE_SEG7 }, { 2,10,28, 1, CORE_SEG7 },
  { 4, 4,33, 1, CORE_SEG7 }, { 4, 8,35, 1, CORE_SEG7 }, {0}
};
INITGAME_S10(pfevr, GEN_S9, dispPfevr, 0, FLIP_SW(FLIP_L), S11_BCDDIAG|S11_BCDDISP, 0x08)
S9_ROMSTART12(pfevr,l2,"pf-rom1.u19", CRC(00be42bd) SHA1(72ca21c96e3ffa3c43499165f3339b669c8e94a5),
                       "pf-rom2.u20", CRC(7b101534) SHA1(21e886d5872104d71bb528b9affb12230268597a))
S9S_SOUNDROM4("cpu_u49.128", CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
S9_ROMEND
S11_INPUT_PORTS_START(pfevr, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(pfevr, l2, "Pennant Fever Baseball (L-2)", 1984, "Williams", s9_mS9PS, 0)

S9_ROMSTART12(pfevr,p3,"cpu_u19.732", CRC(03796c6d) SHA1(38c95fcce9d0f357a74f041f0df006b9c6f6efc7),
                       "cpu_u20.764", CRC(3a3acb39) SHA1(7844cc30a9486f718a556850fc9cef3be82f26b7))
S9S_SOUNDROM4("cpu_u49.128", CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
S9_ROMEND
CORE_CLONEDEF(pfevr, p3, l2, "Pennant Fever Baseball (P-3 Prototype)", 1984, "Williams", s9_mS9PS, 0)

/*--------------------
/ Still Crazy (#534)
/--------------------*/
static const core_tLCDLayout dispCrzy[] = {
  {0, 0,21,7,CORE_SEG87}, {0}
};
INITGAME_S10(scrzy, GEN_S9, dispCrzy, S9_BREAKPIA, FLIP_SW(FLIP_L), S11_BCDDIAG|S11_BCDDISP, 0)
S9_ROMSTARTx2(scrzy,l1,"scrzy_20.bin", CRC(b0df42e6) SHA1(bb10268d7b820d1de0c20e1b79aba558badd072b))
S9S_SOUNDROM4("scrzy_49.bin", CRC(bcc8ccc4) SHA1(2312f9cc4f5a2dadfbfa61d13c31bb5838adf152))
S9_ROMEND
S11_INPUT_PORTS_START(scrzy, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(scrzy, l1, "Still Crazy (L-1)", 1984, "Williams", s9_mS9S, 0)

/*--------------------
/ Strike Zone (#916)
/--------------------*/
static core_tLCDLayout dispSzone[] = {
  { 0, 0,20, 4, CORE_SEG7 }, { 0,12,24, 4, CORE_SEG7 },
  { 2, 0,28, 4, CORE_SEG7 }, { 2,12, 0, 4, CORE_SEG7 },
  { 4, 0, 4, 4, CORE_SEG7 }, { 4,12, 8, 4, CORE_SEG7 },
  { 6, 4,33, 1, CORE_SEG7 }, { 6, 6,32, 1, CORE_SEG7 }, { 6,12,34, 2, CORE_SEG7 }, {0}
};
INITGAME_S10(szone, GEN_S9, dispSzone, 0, FLIP_SW(FLIP_L), S11_BCDDIAG|S11_BCDDISP, 0)
S9_ROMSTART12(szone,l5,"sz_u19r5.732", CRC(c79c46cb) SHA1(422ba74ae67bebbe02f85a9a8df0e3072f3cebc0),
                       "sz_u20r5.764", CRC(9b5b3be2) SHA1(fce051a60b6eecd9bc07273892b14046b251b372))
S9S_SOUNDROM4("szs_u49.128", CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
S9_ROMEND
S11_INPUT_PORTS_START(szone, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(szone, l5, "Strike Zone (Shuffle) (L-5)", 1984, "Williams", s9_mS9S,0)

S9_ROMSTART12(szone,l2,"sz_u19r2.732", CRC(c0e4238b) SHA1(eae60ccd5b5001671cd6d2685fd588494d052d1e),
                       "sz_u20r2.764", CRC(91c08137) SHA1(86da08f346f85810fceceaa7b9824ab76a68da54))
S9S_SOUNDROM4("szs_u49.128", CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
S9_ROMEND
CORE_CLONEDEF(szone,l2,l5,"Strike Zone (Shuffle) (L-2)", 1984, "Williams", s9_mS9S,0)

/*--------------------
/ Alley Cats (#918)
/--------------------*/
// Alley Cats is really S9 (uses a BCD digit for diagnostics) but with S11 sound board
MACHINE_DRIVER_START(s9_mS11S)
  MDRV_IMPORT_FROM(s11_s11S)
  MDRV_DIAGNOSTIC_LED7
MACHINE_DRIVER_END

INITGAME_S10(alcat, GEN_S11, dispS10, 0, FLIP_SW(FLIP_L), S11_BCDDIAG|S11_BCDDISP, 0)
S9_ROMSTART12(alcat,l7,"u26_rev7.rom", CRC(4d274dd3) SHA1(80d72bd0f85ce2cac04f6d9f59dc1fcccc86d402),
                       "u27_rev7.rom", CRC(9c7faf8a) SHA1(dc1a561948b9a303f7924d7bebcd972db766827b))
S11S_SOUNDROM88(       "acs_u21.bin",CRC(c54cd329) SHA1(4b86b10e60a30c4de5d97129074f5657447be676),
                       "acs_u22.bin",CRC(56c1011a) SHA1(c817a3410c643617f3643897b8f529ae78546b0d))
S11_ROMEND
S11_INPUT_PORTS_START(alcat, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(alcat, l7, "Alley Cats (Shuffle) (L-7)", 1985, "Williams", s9_mS11S,0)

/*--------------------
/ Tic-Tac-Strike (#919)
/--------------------*/
INITGAME_S10(tts, GEN_S11, dispS10, 0, FLIP_SW(FLIP_L), S11_BCDDISP, 0)
S9_ROMSTARTx4(tts,l2,"u27_l2.128",CRC(edbcab92) SHA1(0f6b2dc01874984f9a17ee873f2fa0b6c9bba5be))
S11S_SOUNDROM88(       "tts_u21.256", NO_DUMP,
                       "tts_u22.256", NO_DUMP)
S11_ROMEND
S11_INPUT_PORTS_START(tts, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(tts, l2, "Tic-Tac-Strike (Shuffle) (L-2)", 1986, "Williams", s11_mS11S,0)

S9_ROMSTARTx4(tts,l1,"tts_u27.128",CRC(f540c53c) SHA1(1c7a318278ad1afdcbe6aaf81f9b774882b069d6))
S11S_SOUNDROM88(       "tts_u21.256", NO_DUMP,
                       "tts_u22.256", NO_DUMP)
S11_ROMEND
CORE_CLONEDEF(tts, l1, l2, "Tic-Tac-Strike (Shuffle) (L-1)", 1986, "Williams", s11_mS11S,0)

/*--------------------
/ Gold Mine (#920)
/--------------------*/
INITGAME_S10(gmine, GEN_S11, dispS10, 0, FLIP_SW(FLIP_L), S11_BCDDISP, 0)
S9_ROMSTARTx4(gmine,l2,"u27.128",CRC(99c6e049) SHA1(356faec0598a54892050a28857e9eb5cdbf35833))
S11S_SOUNDROM88(       "u21.256",CRC(3b801570) SHA1(50b50ff826dcb031a30940fa3099bd3a8d773831),
                       "u22.256",CRC(08352101) SHA1(a7437847a71cf037a80686292f9616b1e08922df))
S11_ROMEND
S11_INPUT_PORTS_START(gmine, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(gmine, l2, "Gold Mine (Shuffle) (L-2)", 1987, "Williams", s11_mS11S,0)

/*--------------------
/ Top Dawg (#921)
/--------------------*/
INITGAME_S10(tdawg, GEN_S11, dispS10, 0, FLIP_SW(FLIP_L), S11_BCDDISP, 0)
S9_ROMSTARTx4(tdawg,l1,"tdu27r1.128",CRC(0b4bb586) SHA1(a927ebf7167609cc84b38c22aa35d0c4d259dd8b))
S11S_SOUNDROM88(      "tdsu21r1.256",CRC(6a323227) SHA1(7c7263754e5672c654a2ee9582f0b278e637a909),
                      "tdsu22r1.256",CRC(58407eb4) SHA1(6bd9b304c88d9470eae5afb6621187f4a8313573))
S11_ROMEND
S11_INPUT_PORTS_START(tdawg, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(tdawg, l1, "Top Dawg (Shuffle) (L-1)", 1987, "Williams", s11_mS11S,0)

/*--------------------
/ Shuffle Inn (#922)
/--------------------*/
INITGAME_S10(shfin, GEN_S11, dispS10, 0, FLIP_SW(FLIP_L), S11_BCDDISP, 0)
S9_ROMSTARTx4(shfin,l1,"u27rom-1.rv1",CRC(40cfb74a) SHA1(8cee4212ea8bb6b360060391df3208e1e129d7e5))
S11S_SOUNDROM88(       "u21snd-2.rv1",CRC(80ddce05) SHA1(9498260e5ccd2fe0eb03ff321dd34eb945b0213a),
                       "u22snd-2.rv1",CRC(6894abaf) SHA1(2d661765fbfce33a73a20778c41233c0bd9933e9))
S11_ROMEND
S11_INPUT_PORTS_START(shfin, 1) S11_INPUT_PORTS_END
CORE_GAMEDEF(shfin, l1, "Shuffle Inn (Shuffle) (L-1)", 1987, "Williams", s11_mS11S,0)

/*-------------------------------------------------------------------
/ La Rana (1990)
/-------------------------------------------------------------------*/
extern MACHINE_DRIVER_EXTERN(INDERS1RANA);
static core_tLCDLayout inderDispRana[] = {
  {0, 0, 5,3,CORE_SEG7}, {3, 0, 1,3,CORE_SEG7}, {0, 8,13,3,CORE_SEG7}, {3, 8, 9,3,CORE_SEG7},
  {1,16,12,1,CORE_SEG7}, {1,18, 4,1,CORE_SEG7},
  {0}
};
INDER_INPUT_PORTS_START(larana, 1) INDER_INPUT_PORTS_END
static core_tGameData laranaGameData = {0,inderDispRana,{FLIP_SW(FLIP_L),0,8,0,SNDBRD_SPINB}};
static void init_larana(void) {
  core_gameData = &laranaGameData;
}
ROM_START(larana)
  NORMALREGION(0x10000, INDER_MEMREG_CPU)
    ROM_LOAD("game_0_050790.bin", 0x0000,0x2000, CRC(ba94618f) SHA1(0fd6ffe9a6ef514c1dbf8856b881a54bf184e863))
  NORMALREGION(0x10000, INDER_MEMREG_SND)
    ROM_LOAD("sound_a_050690.bin",0x0000,0x2000, CRC(1513fd92) SHA1(6ca0723f5d7c86b844476a4830c8fc3744cbf918))
  NORMALREGION(0x40000, REGION_USER1)
    ROM_LOAD("sound_b_200690.bin",0x0000,0x10000,CRC(3aaa7c7d) SHA1(4a8531b6859fc1f2a4bb63a51da35e9081b7e88b))
ROM_END
CORE_GAMEDEFNV(larana,"La Rana",1990,"Inder (Spain)",INDERS1RANA,0)

// This set gives, by default, 2 launches per coin. Other changes are unknown.
#define init_larana2 init_larana
#define input_ports_larana2 input_ports_larana
ROM_START(larana2)
  NORMALREGION(0x10000, INDER_MEMREG_CPU)
    ROM_LOAD("larana_alt.bin",    0x0000,0x2000, CRC(81a6c8c3) SHA1(e3678e18704d0be0a1e9ece178babec7d728e510))
  NORMALREGION(0x10000, INDER_MEMREG_SND)
    ROM_LOAD("sound_a_050690.bin",0x0000,0x2000, CRC(1513fd92) SHA1(6ca0723f5d7c86b844476a4830c8fc3744cbf918))
  NORMALREGION(0x40000, REGION_USER1)
    ROM_LOAD("sound_b_200690.bin",0x0000,0x10000,CRC(3aaa7c7d) SHA1(4a8531b6859fc1f2a4bb63a51da35e9081b7e88b))
ROM_END
CORE_CLONEDEFNV(larana2,larana,"La Rana (alternate set)",1990,"Inder (Spain)",INDERS1RANA,0)

/*-------------------------------------------------------------------
/ Gun Shot
--------------------------------------------------------------------*/
extern MACHINE_DRIVER_EXTERN(gunshot);
static core_tLCDLayout gs_disp[] = {{0}};
static core_tGameData gunshotGameData = {0,gs_disp,{FLIP_SW(FLIP_L),0,-4,0,SNDBRD_SPINB,0,0,175},NULL,{"",{0,0,0x0f,0x10}}};
static void init_gunshot(void) {
  core_gameData = &gunshotGameData;
}
INPUT_PORTS_START(gunshot)
  CORE_PORTS
  SIM_PORTS(1)
  PORT_START /* 0, switches */
    COREPORT_BIT(     0x0001, "Coin", KEYCODE_1)
    COREPORT_BIT(     0x0008, "Tilt", KEYCODE_DEL)
    COREPORT_BIT(     0x0020, "Fire", KEYCODE_ENTER)
  PORT_START /* 1, dips 1..8 */
    COREPORT_DIPNAME( 0x000f, 0x000a, "Games/Coins")
      COREPORT_DIPSET(0x0000, "2/1" )
      COREPORT_DIPSET(0x0001, "1/1" )
      COREPORT_DIPSET(0x000a, "1/1, 3/2" )
      COREPORT_DIPSET(0x0002, "1/1, 5/4" )
      COREPORT_DIPSET(0x000f, "2/2, 5/4" )
      COREPORT_DIPSET(0x0003, "1/2" )
      COREPORT_DIPSET(0x0004, "1/2, 5/8" )
      COREPORT_DIPSET(0x0005, "1/2, 3/4" )
      COREPORT_DIPSET(0x0006, "1/4" )
      COREPORT_DIPSET(0x0007, "1/4, 6/20" )
      COREPORT_DIPSET(0x0008, "1/4, 3/8" )
//    COREPORT_DIPSET(0x0009, "1/1" )
//    COREPORT_DIPSET(0x000b, "1/2" )
//    COREPORT_DIPSET(0x000c, "1/2, 3/4" )
//    COREPORT_DIPSET(0x000d, "1/2, 3/4" )
//    COREPORT_DIPSET(0x000e, "1/2, 3/4" )
    COREPORT_DIPNAME( 0x0010, 0x0000, DEF_STR(Unknown))
      COREPORT_DIPSET(0x0000, DEF_STR(Off))
      COREPORT_DIPSET(0x0010, DEF_STR(On))
    COREPORT_DIPNAME( 0x0020, 0x0020, "Attract tune")
      COREPORT_DIPSET(0x0000, DEF_STR(Off))
      COREPORT_DIPSET(0x0020, DEF_STR(On))
    COREPORT_DIPNAME( 0x0040, 0x0000, "Test mode")
      COREPORT_DIPSET(0x0000, DEF_STR(Off))
      COREPORT_DIPSET(0x0040, DEF_STR(On))
    COREPORT_DIPNAME( 0x0080, 0x0000, "Game mode")
      COREPORT_DIPSET(0x0000, "Normal" )
      COREPORT_DIPSET(0x0080, "Exhibition (endless)" )
INPUT_PORTS_END
ROM_START(gunshot)
  NORMALREGION(0x10000, REGION_CPU1)
    ROM_LOAD("m-177_gun_shot_rom_0_version_0.4.ci3", 0x0000, 0x4000, CRC(4754a983) SHA1(21c517c78624af41e0295877cc6d6ba7a66fe0fa))
  NORMALREGION(0x10000, REGION_CPU2)
    ROM_LOAD("m-177_gun_shot_rom_1_version_0.0.ic9", 0x0000, 0x2000, CRC(19fcec2d) SHA1(ff69de0652a54d5c00738c8144897e33df972eee))
  NORMALREGION(0x80000, REGION_USER1)
    ROM_LOAD("m-177_gun_shot_rom_2_version_0.0.ic16",0x00000,0x80000,CRC(f91ddd0c) SHA1(cc4e1440e76330872f512d56376f45b92a8dbee6))
ROM_END
CORE_GAMEDEFNV(gunshot, "Gun Shot", 1996, "Spinball (Spain)", gunshot, 0) 

/*--------------------
/ Slugfest baseball
/--------------------*/
static core_tGameData sfGameData = {
  GEN_WPCDMD, wpc_dispDMD,
  { 0 },
  NULL,
  { "",
    /*Coin    1     2     3     4     5     6     7     8     9    10   Cab.  Cust */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 13, 14, 21, 22, 0 }
  }
};
static void init_sf(void) { core_gameData = &sfGameData; }
WPC_ROMSTART(sf,l1,"sf_u6.l1",0x40000,CRC(ada93967) SHA1(90094d207dafdacfaf7d259c6cc3dc2b552c8588))
WPCS_SOUNDROM222("sf_u18.l1",CRC(78092c83) SHA1(7c922dfd8be4bb5e23d4c86b6eb18a29cc034338),
                 "sf_u15.l1",CRC(adcaeaa1) SHA1(27aa9526c628634c395161f4966d9943bdf1f120),
                 "sf_u14.l1",CRC(b830b419) SHA1(c59980a78d8cb1d979de21dfc5ad3d671d8486e7))
WPC_ROMEND
WPC_ROMSTART(sf,d1,"sf_u6.d1",0x40000,CRC(9cf39b6a) SHA1(213edb85f9f71852824e480fd3673ede97705b21))
WPCS_SOUNDROM222("sf_u18.l1",CRC(78092c83) SHA1(7c922dfd8be4bb5e23d4c86b6eb18a29cc034338),
                 "sf_u15.l1",CRC(adcaeaa1) SHA1(27aa9526c628634c395161f4966d9943bdf1f120),
                 "sf_u14.l1",CRC(b830b419) SHA1(c59980a78d8cb1d979de21dfc5ad3d671d8486e7))
WPC_ROMEND
WPC_INPUT_PORTS_START(sf, 0) WPC_INPUT_PORTS_END
CORE_GAMEDEF(sf,l1,"Slugfest (L-1)",1991,"Williams",wpc_mDMDS,0)
CORE_CLONEDEF(sf,d1,l1,"Slugfest (D-1 LED Ghost Fix)",1991,"Williams",wpc_mDMDS,0)

/*-------------------
/ Strike Master
/--------------------*/
static core_tGameData strikGameData = {
  GEN_WPCFLIPTRON, wpc_dispDMD,
  { 0 },
  NULL,
  { "",
    { 0 }, /* No inverted switches */
    { 13, 14, 21, 22, 0 }
  }
};
static void init_strik(void) { core_gameData = &strikGameData; }
WPC_ROMSTART(strik,l4,"strik_l4.rom",0x40000,CRC(c99ea24c) SHA1(f8b083adcbabdc70a1bf7c87c9b488eca7b1c788))
WPCS_SOUNDROM222("lc_u18.l1",CRC(beb84fd9) SHA1(b1d5472af5e3c0f5c67e7d636122eb79e02494ba),
                 "lc_u15.l1",CRC(25fe0be3) SHA1(a784b99daab1255487d4cb05d008a8aee0c39b30),
                 "lc_u14.l1",CRC(7b6dd395) SHA1(fdb01f70bb5f1a4ada9805770e544c4a191ddf26))
WPC_ROMEND
WPC_ROMSTART(strik,d4,"strik_d4.rom",0x40000,CRC(3c93ac35) SHA1(9eff61027a50999c9f2e8c07240c9cc352b3b61b))
WPCS_SOUNDROM222("lc_u18.l1",CRC(beb84fd9) SHA1(b1d5472af5e3c0f5c67e7d636122eb79e02494ba),
                 "lc_u15.l1",CRC(25fe0be3) SHA1(a784b99daab1255487d4cb05d008a8aee0c39b30),
                 "lc_u14.l1",CRC(7b6dd395) SHA1(fdb01f70bb5f1a4ada9805770e544c4a191ddf26))
WPC_ROMEND
WPC_INPUT_PORTS_START(strik, 0) WPC_INPUT_PORTS_END
CORE_GAMEDEF(strik,l4,"Strike Master (L-4)",1992,"Williams",wpc_mFliptronS,0)
CORE_CLONEDEF(strik,d4,l4,"Strike Master (D-4 LED Ghost Fix)",1992,"Williams",wpc_mFliptronS,0)

/*------------------
/ Hot Shot (#60017)
/------------------*/
static core_tGameData hshotGameData = {
  GEN_WPCFLIPTRON, wpc_dispDMD,
  { FLIP_SW(FLIP_L | FLIP_U) | FLIP_SOL(FLIP_L) },
  NULL,
  {
    "905  123456 12345 123",
    /*Coin    1     2     3     4     5     6     7     8     9    10   Cab.  Cust */
    { 0x00, 0x00, 0x20, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    /*Start  Tilt  SlamTilt  CoinDoor  Shooter */
    { 13,    0,    21,       22,       26 }
  }
};
static void init_hshot(void) { core_gameData = &hshotGameData; }
WPC_ROMSTART(hshot,p8,"hshot_p8.u6",0x80000,CRC(26dd6bb2) SHA1(45674885052838b6bd6b3ed0a276a4d9323290c5))
WPCS_SOUNDROM2x8("hshot_l1.u18",CRC(a0e5beba) SHA1(c54a22527d861df54891308752ebdec5829deceb),
                 "hshot_l1.u14",CRC(a3ccf557) SHA1(a8e518ea115cd1963544273c45d9ae9a6cab5e1f))
WPC_ROMEND
WPC_INPUT_PORTS_START(hshot, 0) WPC_INPUT_PORTS_END
CORE_GAMEDEF(hshot,p8,"Hot Shot Basketball (P-8 Prototype)",1992,"Midway",wpc_mFliptronS,0)

WPC_ROMSTART(hshot,p9,"hshot_p9.u6",0x80000,CRC(fa4d05df) SHA1(235aa096f2983f77ade6b257fd8544d91b5f6b28))
WPCS_SOUNDROM2x8("hshot_l1.u18",CRC(a0e5beba) SHA1(c54a22527d861df54891308752ebdec5829deceb),
                 "hshot_l1.u14",CRC(a3ccf557) SHA1(a8e518ea115cd1963544273c45d9ae9a6cab5e1f))
WPC_ROMEND
CORE_CLONEDEF(hshot,p9,p8,"Hot Shot Basketball (P-9 LED Ghost Fix)",1992,"Midway",wpc_mFliptronS,0)

/*-------------
/ Addams Family Values
/--------------*/
static core_tGameData afvGameData = {
  GEN_WPCDCS, wpc_dispDMD,
  { FLIP_SW(FLIP_L | FLIP_U) | FLIP_SOL(FLIP_L) },
  NULL,
  {
    "",
    /*Coin    1     2     3     4     5     6     7     8     9    10   Cab.  Cust */
    { 0x00, 0x00, 0x00, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    /*Start  Tilt  SlamTilt  CoinDoor  Shooter */
    { 13,    14,   21,       22 }
  }
};
static void init_afv(void) { core_gameData = &afvGameData; }
WPC_ROMSTART(afv,l4,"afv_u6.l4",0x80000,CRC(37369339) SHA1(e44a91faca80ffa00d6db78e2df7aa9bf14e957c))
DCS_SOUNDROM1x("afv_su2.l1",CRC(1aa878fc) SHA1(59a89071001b5da6ab56d691721a015773f5f0b5))
WPC_ROMEND
WPC_ROMSTART(afv,d4,"afv_u6.d4",0x80000,CRC(0b6c7c3c) SHA1(e389ab1f3aaecebc852b4cca74870ee939566f8c))
DCS_SOUNDROM1x("afv_su2.l1",CRC(1aa878fc) SHA1(59a89071001b5da6ab56d691721a015773f5f0b5))
WPC_ROMEND

INPUT_PORTS_START(afv)
  CORE_PORTS
  SIM_PORTS(1)
  PORT_START /* 0 */ \
    /* These go into column 0 */ \
    COREPORT_BIT(     0x0001, "Coin Drop",      KEYCODE_5) \
    COREPORT_BIT(     0x0010, "Enter",          KEYCODE_7) \
    COREPORT_BIT(     0x0020, "Up",             KEYCODE_8) \
    COREPORT_BIT(     0x0040, "Down",           KEYCODE_9) \
    COREPORT_BIT(     0x0080, "Escape",         KEYCODE_0) \
    /* Common switches */ \
    COREPORT_BITTOG(  0x0100, "Coin Door",      KEYCODE_END)  \
    COREPORT_BIT   (  0x0200, "Punch Button",   KEYCODE_1)  \
    COREPORT_BITDEF(  0x0400, IPT_TILT,         KEYCODE_INSERT)  \
    COREPORT_BIT(     0x0800, "Slam Tilt",      KEYCODE_HOME)  \
  PORT_START /* 1 */ \
    COREPORT_DIPNAME( 0x0001, 0x0001, "SW1") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0001, "1" ) \
    COREPORT_DIPNAME( 0x0002, 0x0002, "SW2") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0002, "1" ) \
    COREPORT_DIPNAME( 0x0004, 0x0004, "SW3") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0004, "1" ) \
    COREPORT_DIPNAME( 0x0008, 0x0008, "SW4") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0008, "1" ) \
    COREPORT_DIPNAME( 0x0010, 0x0010, "SW5") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0010, "1" ) \
    COREPORT_DIPNAME( 0x0020, 0x0020, "SW6") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0020, "1" ) \
    COREPORT_DIPNAME( 0x0040, 0x0040, "SW7") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0040, "1" ) \
    COREPORT_DIPNAME( 0x0080, 0x0080, "SW8") \
      COREPORT_DIPSET(0x0000, "0" ) \
      COREPORT_DIPSET(0x0080, "1" ) \
INPUT_PORTS_END
CORE_GAMEDEF(afv,l4,"Addams Family Values (Coin Dropper) (L-4)",1993,"Williams",wpc_mDCSS,0)
CORE_CLONEDEF(afv,d4,l4,"Addams Family Values (Coin Dropper) (D-4 LED Ghost Fix)",1993,"Williams",wpc_mDCSS,0)

/*-------------------------------------------------------------------
/ Strikes N' Spares (#N111)
/-------------------------------------------------------------------*/
static struct core_dispLayout GTS3_dispDMD[] = {
  {0,0,32,128,CORE_DMD|CORE_DMDNOAA,(genf *)gts3_dmd128x32a},
  {34,0,32,128,CORE_DMD|CORE_DMDNOAA,(genf *)gts3_dmd128x32b},
  {0}
};
static core_tGameData snsparesGameData = {GEN_GTS3,GTS3_dispDMD,{FLIP_SWNO(21,22),4,4,0,SNDBRD_NONE,0}};
static void init_snspares(void) { core_gameData = &snsparesGameData; }
GTS3ROMSTART(snspares,  "gprom.bin", CRC(9e018496) SHA1(a4995f153ba2179198cfc56b7011707328e4ec89))
GTS3_DMD256_ROMSTART2(  "dsprom.bin",CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
NORMALREGION(0x100000, REGION_USER3) \
  GTS3S_ROMLOAD4(0x00000, "arom1.bin", CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb)) \
GTS3_ROMEND
GTS32_INPUT_PORTS_START(snspares, 4) GTS3_INPUT_PORTS_END
CORE_GAMEDEFNV(snspares,"Strikes N' Spares (rev. 6)",1995,"Gottlieb",mGTS3DMDS2, 0)

#define init_snspare1 init_snspares
#define input_ports_snspare1 input_ports_snspares
GTS3ROMSTART(snspare1,  "gprom1.bin",CRC(590393f4) SHA1(f52400c620e510253abd1c0719050b9bb09be942))
GTS3_DMD256_ROMSTART2(  "dsprom.bin",CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
NORMALREGION(0x100000, REGION_USER3) \
  GTS3S_ROMLOAD4(0x00000, "arom1.bin", CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb)) \
GTS3_ROMEND
CORE_CLONEDEFNV(snspare1,snspares,"Strikes N' Spares (rev. 1)",1995,"Gottlieb",mGTS3DMDS2, 0)

#define init_snspare2 init_snspares
#define input_ports_snspare2 input_ports_snspares
GTS3ROMSTART(snspare2,  "gprom2.bin",CRC(79906dfc) SHA1(1efb68dd391f79e6f8ad5a588145d6ad7b36743c))
GTS3_DMD256_ROMSTART2(  "dsprom.bin",CRC(5c901899) SHA1(d106561b2e382afdb16e938072c9c8f1d1ccdae6))
NORMALREGION(0x100000, REGION_USER3) \
  GTS3S_ROMLOAD4(0x00000, "arom1.bin", CRC(e248574a) SHA1(d2bdc2b9a330bb81556d25d464f617e0934995eb)) \
GTS3_ROMEND
CORE_CLONEDEFNV(snspare2,snspares,"Strikes N' Spares (rev. 2)",1995,"Gottlieb",mGTS3DMDS2, 0)

/*-----------------------------
/ League Champ (Shuffle Alley)
/------------------------------*/
static core_tGameData lcGameData = {
  GEN_WPCFLIPTRON, wpc_dispDMD,
  { 0 },
  NULL,
  {
    "",
    { 0 }, /* No inverted switches */
    { 13, 14, 21, 22, 0 }
  }
};
static void init_lc(void) { core_gameData = &lcGameData; }
WPC_ROMSTART(lc,11,"lchmp1_1.rom",0x80000,CRC(60ab944c) SHA1(d2369b0a864e864b269de1765121e4534fa8fa59))
WPCS_SOUNDROM222("lc_u18.l1",CRC(beb84fd9) SHA1(b1d5472af5e3c0f5c67e7d636122eb79e02494ba),
                 "lc_u15.l1",CRC(25fe0be3) SHA1(a784b99daab1255487d4cb05d008a8aee0c39b30),
                 "lc_u14.l1",CRC(7b6dd395) SHA1(fdb01f70bb5f1a4ada9805770e544c4a191ddf26))
WPC_ROMEND
WPC_INPUT_PORTS_START(lc, 0) WPC_INPUT_PORTS_END
CORE_GAMEDEF(lc,11,"League Champ (1.1)",1996,"Bally",wpc_mFliptronS,0)

/*-------------------------------------------------------------------
/ Cut The Cheese (Redemption, Data East hardware)
/-------------------------------------------------------------------*/
static core_tGameData ctcGameData = {
  GEN_DEDMD64, de_dmd192x64, {0,0,0,0,SNDBRD_DE2S,SNDBRD_DEDMD64}, NULL, {{0}},{10}
};
static void init_ctcheese(void) { core_gameData = &ctcGameData; }
DE_ROMSTARTx0(ctcheese,"ctcc5.bin",CRC(465d41de) SHA1(0e30b527d5b47f8823cbe6f196052b090e69e907))
DE_DMD64ROM88("ctcdsp0.bin", CRC(6885734d) SHA1(9ac82c9c8bf4e66d2999fbfd08617ef6c266dfe8),
              "ctcdsp3.bin", CRC(0c2b3f3c) SHA1(cb730cc6fdd2a2786d25b46b1c45466ee56132d1))
DE2S_SOUNDROM144("ctcsnd.u7", CRC(406b9b9e) SHA1(f3f86c368c92ee0cb47323e6e0ca0fa05b6122bd),
                 "ctcsnd.u17", CRC(ea125fb3) SHA1(2bc1d2a6138ff77ad19b7bcff784dba73f545883),
                 "ctcsnd.u21", CRC(1b3af383) SHA1(c6b57f3f0781954f75d164d909093e4ed8da440e))
SE_ROMEND

DE_INPUT_PORTS_START2(ctcheese, 1) DE_INPUT_PORTS_END
CORE_GAMEDEFNV(ctcheese,"Cut The Cheese (Redemption)",1996,"Sega",de_mDEDMD64S2A,GAME_NOT_WORKING)

/*-------------------------------------------------------------------
/ Cut The Cheese Deluxe (Redemption, Whitestar hardware)
/-------------------------------------------------------------------*/
static core_tGameData ctcdlxGameData = {
  GEN_WS, se_dmd128x32, {0,0,2}
};
static void init_ctchzdlx(void) { core_gameData = &ctcdlxGameData; }
SE128_ROMSTART(ctchzdlx,"ctcdxcpu.100",CRC(faad6656) SHA1(4d868bc31f35e848424e3bb66cb87efe0cf24eca))
DE_DMD32ROM8x(   "ctcdxdsp.100",CRC(de61b12e) SHA1(2ef8f02ca995e67d1feebd33306f92e885077101))
DE2S_SOUNDROM144("ctcu7d.bin", CRC(92bfe454) SHA1(8182f7ac84addf8bdb7976a85c801edf3424d16b),
                 "ctcu17.bin", CRC(7ee35d17) SHA1(f2c9b70285926fc782a2e1289532395cd8dbf999),
                 "ctcu21.bin", CRC(84dd40ac) SHA1(c9327b95f1730a3aa741540c28078f214af214b8))
SE_ROMEND

SE_INPUT_PORTS_START(ctchzdlx, 1) SE_INPUT_PORTS_END
CORE_CLONEDEFNV(ctchzdlx,ctcheese,"Cut The Cheese Deluxe (Redemption)",1998,"Sega",de_mSES1,0)

/*-------------------------------------------------------------------
/ Wack-A-Doodle-Doo (Redemption)
/-------------------------------------------------------------------*/
static core_tGameData wackGameData = {
  GEN_WS, se_dmd128x32, {0,0,2}
};
static void init_wackadoo(void) { core_gameData = &wackGameData; }
SE128_ROMSTART(wackadoo,"wackcpu.100",CRC(995e0219) SHA1(57b2352a5a96e71ff48f838fde87a158afbf3701))
DE_DMD32ROM8x(   "wackdisp.bin",CRC(8b46f074) SHA1(38a98fff56186d1a152e2c2e321496fd24e9ad56))
DE2S_SOUNDROM144("wacku7.bin",  CRC(99df3022) SHA1(5a38cf0816b95ea399d659d724b3259fadb42a62),
                 "wacku17.bin", CRC(1876e659) SHA1(1ff3a6d7a7b2582d0482eae024efa078d0a61dbe),
                 "wacku21.bin", CRC(a3207296) SHA1(19af8ebab298a353fe42fc297b61182d8f772347))
SE_ROMEND
SE_INPUT_PORTS_START(wackadoo, 1) SE_INPUT_PORTS_END
CORE_GAMEDEFNV(wackadoo,"Wack-A-Doodle-Doo (Redemption)",1998,"Sega",de_mSES1,0)

/*-------------------------------------------------------------------
/ Titanic (Coin dropper)
/-------------------------------------------------------------------*/
static core_tGameData coinGameData = {
  GEN_WS, se_dmd128x32, {0, 0, 34, 0, 0, SE_LED2}
};
static void init_titanic(void) { core_gameData = &coinGameData; }
SE128_ROMSTART(titanic, "titacpu.101",CRC(4217becf) SHA1(8b7aacbe75717f13623f6ceaa4ba2de61b1b732a))
DE_DMD32ROM8x(   "titadspa.101",CRC(5b4300b6) SHA1(c2b2de20f6c74b71d2a9ac43c17694eadd795586))
DE2S_SOUNDROM144("titau7.101" ,CRC(544fe1ac) SHA1(5c62eef6a42660b13e626d1a6bb8cd09b77b0cc1),
                 "titau17.100",CRC(ab919e99) SHA1(8735b632a62d9cd3db26b3c832785c48552ba231),
                 "titau21.100",CRC(76ca05f8) SHA1(3e1c56fe37393c345111665fd8ab730d53cb6970))
SE_ROMEND
SE_INPUT_PORTS_START(titanic, 1) SE_INPUT_PORTS_END
CORE_GAMEDEFNV(titanic,"Titanic (Coin Dropper)",1998,"Sega",de_mSES2T,0)

/*-------------------------------------------------------------------
/ Monopoly (Coin dropper)
/-------------------------------------------------------------------*/
static void init_monopred(void) { core_gameData = &coinGameData; }
SE128_ROMSTART(monopred, "monopcpu.401",CRC(2d0ff130) SHA1(c0b7baa6973db8743186a950b0a1ad3383db5c98))
DE_DMD32ROM8x(  "monopdsp.400",CRC(0105572b) SHA1(fd9e1dfa4f396b953f82ccde11b54ee638382bee))
DE2S_SOUNDROM18("monopred.u7" ,CRC(1ca0cf63) SHA1(c4ce78718e3e3f1a8451b134f2869dd6410fee30),
                "monopred.u17",CRC(467dca62) SHA1(c727748b6b0b39ead19ce98bddd89fd05fb62d00))
SE_ROMEND
SE_INPUT_PORTS_START(monopred, 1) SE_INPUT_PORTS_END
CORE_GAMEDEFNV(monopred,"Monopoly (Coin Dropper)",2002,"Stern",de_mSES1,0)
