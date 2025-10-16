#include "driver.h"
#include "machine/6821pia.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6502/m6502.h"
#include "cpu/i8039/i8039.h"
#include "sound/discrete.h"
#include "sound/sn76477.h"
#include "sound/tms5220.h" // TMS5200 & TMS5220CNL
#include "core.h"
#include "sndbrd.h"
#include "zacsnd.h"

extern WRITE_HANDLER(UpdateZACSoundLED);
extern void UpdateZACSoundACT(int data);

/*----------------------------------------
/ Zaccaria Sound-On-Board 1311
/ 4 simple tone generators, solenoid-controlled
/-----------------------------------------*/
static DISCRETE_SOUND_START(zac1311_discInt)
  DISCRETE_INPUT(NODE_01,1,0x000f,0)                         // Input handlers, mostly for enable
  DISCRETE_INPUT(NODE_02,2,0x000f,0)
  DISCRETE_INPUT(NODE_04,4,0x000f,0)
  DISCRETE_INPUT(NODE_08,8,0x000f,0)

  DISCRETE_SAWTOOTHWAVE(NODE_10,NODE_01,349,50000,10000,0,0) // F note
  DISCRETE_SAWTOOTHWAVE(NODE_20,NODE_02,440,50000,10000,0,0) // A note
  DISCRETE_SAWTOOTHWAVE(NODE_30,NODE_04,523,50000,10000,0,0) // C' note
  DISCRETE_SAWTOOTHWAVE(NODE_40,NODE_08,698,50000,10000,0,0) // F' note

  DISCRETE_ADDER4(NODE_50,1,NODE_10,NODE_20,NODE_30,NODE_40) // Mix all four sound sources

  DISCRETE_OUTPUT(NODE_50, 50)                               // Take the output from the mixer
DISCRETE_SOUND_END

MACHINE_DRIVER_START(zac1311)
  MDRV_SOUND_ADD(DISCRETE, zac1311_discInt)
MACHINE_DRIVER_END

const struct sndbrdIntf zac1311Intf = {0};

/*----------------------------------------
/ Zaccaria Sound Board 1125
/ SN76477 sound chip, no CPU
/-----------------------------------------*/
static void zac1125_init(struct sndbrdData *brdData);
static WRITE_HANDLER(zac1125_data_w);

static struct SN76477interface zac1125_sn76477Int = { 1, { 50 }, /* mixing level */
/*                       pin description      */
  { RES_K(47)   },    /*  4  noise_res        */
  { RES_K(220)  },    /*  5  filter_res       */
  { CAP_N(2.2)  },    /*  6  filter_cap       */
  { RES_M(1.5)  },    /*  7  decay_res        */
  { CAP_U(2.2)  },    /*  8  attack_decay_cap */
  { RES_K(4.7)  },    /* 10  attack_res       */
  { RES_K(47)   },    /* 11  amplitude_res    */
  { RES_K(320)  },    /* 12  feedback_res     */
  { 0 /* ??? */ },    /* 16  vco_voltage      */
  { CAP_U(0.33) },    /* 17  vco_cap          */
  { RES_K(100)  },    /* 18  vco_res          */
  { 5.0         },    /* 19  pitch_voltage    */
  { RES_M(1)    },    /* 20  slf_res          */
  { CAP_U(2.2)  },    /* 21  slf_cap          */
  { CAP_U(2.2)  },    /* 23  oneshot_cap      */
  { RES_M(1.5)  }     /* 24  oneshot_res      */
};

/*-------------------
/ exported interface
/--------------------*/
const struct sndbrdIntf zac1125Intf = {
  "ZAC1125", zac1125_init, NULL, NULL, zac1125_data_w, zac1125_data_w, NULL, NULL, NULL, SNDBRD_NODATASYNC|SNDBRD_NOCTRLSYNC
};

MACHINE_DRIVER_START(zac1125)
  MDRV_SOUND_ADD(SN76477, zac1125_sn76477Int)
MACHINE_DRIVER_END

static struct {
  //UINT8 ctrl;
  double ne555_voltage;
  mame_timer *ne555;
} s1125locals;

static void ne555_timer(int n) {
  s1125locals.ne555_voltage += 0.02;
  SN76477_set_vco_voltage(0, s1125locals.ne555_voltage);
  if (s1125locals.ne555_voltage > 4.99) // stop timer if maximum voltage is reached
    timer_adjust(s1125locals.ne555, TIME_NEVER, 0, TIME_NEVER);
}

static WRITE_HANDLER(zac1125_data_w) {
  static const double states[8][5] = { // pins 7, 18, 20, 24, and optional timer interval
    { RES_K(468), RES_K(100),RES_M(1), RES_K(9.9) },
    { RES_K(4.7), RES_K(32), RES_M(1), RES_K(136) },
    { RES_K(468), RES_K(25), RES_M(1), RES_K(9.9) },
    { RES_K(468), RES_K(32), RES_M(1), RES_K(9.9) },
    { RES_K(192), RES_K(4.5),RES_M(1), RES_M(1.5), 0.015 },
    { RES_K(26.5),RES_K(4.5),RES_M(1), RES_K(9.9), 0.0003 },
    { RES_M(1.5), RES_K(100),RES_M(1), RES_K(358) },
    { RES_K(192), RES_K(9.1),RES_K(32),RES_K(94)  }
  };
  static const int vco[8] = { 1, 1, 1, 1, 0, 0, 1, 1 };
  static const int mix[8] = { 0, 0, 0, 0, 0, 0, 2, 0 };

  data &= 0x0f;
  if (data) {
    int state = data >> 1;
    SN76477_enable_w       (0, 1);
    logerror("Sound %x plays\n", state);
    SN76477_mixer_w        (0, mix[state]);
    SN76477_set_decay_res  (0, states[state][0]); /* 7 */
    SN76477_set_vco_res    (0, states[state][1]); /* 18 */
    SN76477_set_slf_res    (0, states[state][2]); /* 20 */
    SN76477_set_oneshot_res(0, states[state][3]); /* 24 */
    SN76477_vco_w          (0, vco[state]);
    if (!vco[state]) { // simulate the loading of the capacitors by using a timer
      SN76477_set_vco_voltage(0, 0);
      s1125locals.ne555_voltage = 0;
      timer_adjust(s1125locals.ne555, states[state][4], 0, states[state][4]);
    }
    SN76477_enable_w       (0, 0);
  }
}

static void zac1125_init(struct sndbrdData *brdData) {
  if (s1125locals.ne555) timer_remove(s1125locals.ne555);
  memset(&s1125locals, 0, sizeof(s1125locals));
  /* MIXER A & C = GND */
  SN76477_mixer_w(0, 0);
  /* ENVELOPE is constant: pin1 = hi, pin 28 = lo */
  SN76477_envelope_w(0, 1);
  /* fake: pulse the enable line to get rid of the constant noise */
//  SN76477_enable_w(0, 1);
//  SN76477_enable_w(0, 0);
  s1125locals.ne555 = timer_alloc(ne555_timer);
}

/*----------------------------------------
/ Zaccaria Sound Board 1346
/ i8035 MCU, no PIAs
/ and sound board 1146
/ with additional SN76477 chip used on Locomotion only
/-----------------------------------------*/
static void sp_init(struct sndbrdData *brdData);
static void sp_diag(int button);
static WRITE_HANDLER(sp1346_data_w);

/*-------------------
/ exported interface
/--------------------*/
const struct sndbrdIntf zac1346Intf = {
  "ZAC1346", sp_init, NULL, sp_diag, sp1346_data_w, sp1346_data_w, NULL, NULL, NULL, 0
};
static struct DACinterface sp1346_dacInt = { 1, { 25 }};

static struct {
  struct sndbrdData brdData;
  int lastcmd;
  int tc;
  int currentValue;
} splocals;

static void sp_init(struct sndbrdData *brdData) {
  splocals.brdData = *brdData;
}

static void sp_diag(int button) {
  cpu_set_irq_line(ZACSND_CPUA, 0, button ? ASSERT_LINE : CLEAR_LINE);
}

static WRITE_HANDLER(sp1346_data_w) {
  logerror("Sound %x plays\n", data);
  splocals.tc = data ? -1 : 0;
  i8035_set_reg(I8035_TC, splocals.tc);

  if (Machine->drv->sound[1].sound_type) { // >0 means SN74677 & NE555 chips present
    static const double res[8] = { 0, RES_M(4.7), RES_M(2), RES_K(820), RES_K(680), RES_K(560), RES_K(470), RES_K(330) };
    switch (data) {
      case 0:
        if (splocals.lastcmd == 0) {
          SN76477_mixer_w(0, 7);
          splocals.currentValue = 0;
        }
        break;
      case 1: // raise the frequency if value < 7
        if (splocals.currentValue < 7) splocals.currentValue++;
        break;
      case 2: // lower the frequency if value > 1
        if (splocals.currentValue > 1) splocals.currentValue--;
        break;
    }
    if (splocals.currentValue > 0) {
      SN76477_mixer_w(0, 4);
      SN76477_set_slf_res(0, res[splocals.currentValue]);
    }
    SN76477_enable_w(0, !splocals.currentValue);
    discrete_sound_w(1, data == 3); // enable the NE555 tone
  }

  splocals.lastcmd = data;
}

static MEMORY_READ_START(i8035_readmem)
  { 0x0000, 0x07ff, MRA_ROM },
  { 0x0800, 0x087f, MRA_RAM },
  { 0x0880, 0x08ff, MRA_RAM },
MEMORY_END

static MEMORY_WRITE_START(i8035_writemem)
  { 0x0800, 0x087f, MWA_RAM },
MEMORY_END

static WRITE_HANDLER(ram_w) {
  UINT8 *rom = memory_region(ZACSND_CPUAREGION);
  rom[0x800 + offset] = data;
}
static READ_HANDLER(ram_r) {
  UINT8 *rom = memory_region(ZACSND_CPUAREGION);
  return rom[0x800 + offset];
}
static READ_HANDLER(sp1346_data_r) { return (core_getDip(0) << 4) | splocals.lastcmd; }
static READ_HANDLER(test_r) { return splocals.tc; }
static WRITE_HANDLER(dac_w) { DAC_0_data_w(0, core_revbyte(data)); }

static PORT_READ_START(i8035_readport)
  { 0x00, 0x7f, ram_r },
  { 0x80, 0xff, sp1346_data_r },
  { I8039_t1, I8039_t1, test_r },
MEMORY_END

static PORT_WRITE_START(i8035_writeport)
  { 0x00, 0x7f, ram_w },
  { I8039_p1, I8039_p1, dac_w },
MEMORY_END

MACHINE_DRIVER_START(zac1346)
  MDRV_CPU_ADD_TAG("scpu", I8035, 6000000./15.) // 8035 has internal divider by 15!
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(i8035_readmem, i8035_writemem)
  MDRV_CPU_PORTS(i8035_readport, i8035_writeport)

  MDRV_SOUND_ADD(DAC, sp1346_dacInt)
  MDRV_INTERLEAVE(500)
MACHINE_DRIVER_END

static struct SN76477interface zac1146_sn76477Int = { 1, { 30 }, /* mixing level */
/*                       pin description      */
  { RES_K(39)   },    /*  4  noise_res        */
  { RES_K(100)  },    /*  5  filter_res       */
  { CAP_P(470)  },    /*  6  filter_cap       */
  { RES_M(1)    },    /*  7  decay_res        */
  { CAP_U(22)   },    /*  8  attack_decay_cap */
  { RES_K(4.7)  },    /* 10  attack_res       */
  { RES_K(47)   },    /* 11  amplitude_res    */
  { RES_K(87.7) },    /* 12  feedback_res     */
  { 0           },    /* 16  vco_voltage      */
  { 0           },    /* 17  vco_cap          */
  { 0           },    /* 18  vco_res          */
  { 5.0         },    /* 19  pitch_voltage    */
  { 0           },    /* 20  slf_res          */
  { CAP_N(220)  },    /* 21  slf_cap          */
  { CAP_U(1)    },    /* 23  oneshot_cap      */
  { RES_K(330)  }     /* 24  oneshot_res      */
};

static int type[1] = {0};
static DISCRETE_SOUND_START(zac1146_discInt)
  DISCRETE_INPUT(NODE_01,1,0x0003,0)
  DISCRETE_555_ASTABLE(NODE_10,NODE_01,12.0,RES_K(1),RES_K(56),CAP_N(10),NODE_NC,type)
  DISCRETE_GAIN(NODE_20,NODE_10,1250)
  DISCRETE_OUTPUT(NODE_20, 50)
DISCRETE_SOUND_END

MACHINE_DRIVER_START(zac1146)
  MDRV_IMPORT_FROM(zac1346)
  MDRV_SOUND_ADD(SN76477, zac1146_sn76477Int)
  MDRV_SOUND_ADD(DISCRETE, zac1146_discInt)
MACHINE_DRIVER_END

/*----------------------------------------
/ Zaccaria Sound & Speech Board 1B1370
/ (almost identical to BY61, TMS5200 chip)
/ and Zaccaria Sound & Speech Board 1B13136
/ (like 1B1370, with an additional 6802 CPU, DAC, and AY8910 chip).
/ It's reported that the sound board is missing the 2nd CPU & ROM chips,
/ so in reality it acts exactly like the older 1370 board
/ with a slightly different memory map (but it can be integrated).
/-----------------------------------------*/
#define SNS_PIA0 0
#define SNS_PIA1 1
#define SNS_PIA2 2

#define SW_TRUE 1

static void sns_init(struct sndbrdData *brdData);
static void sns_diag(int button);
static WRITE_HANDLER(sns_data_w);
static void sns_5220Irq(int state);
static void sns_5220Rdy(int state);
static READ_HANDLER(sns_8910a_r);
static WRITE_HANDLER(sns_8910b_w);
static READ_HANDLER(sns2_8910a_r);
static WRITE_HANDLER(sns_dac_w);

const struct sndbrdIntf zac1370Intf = {
  "ZAC1370", sns_init, NULL, sns_diag, sns_data_w, sns_data_w, NULL, NULL, NULL, SNDBRD_NODATASYNC|SNDBRD_NOCTRLSYNC
};

static struct TMS5220interface sns_tms5220Int = { 640000, 75, sns_5220Irq, sns_5220Rdy }; // the frequency may vary by up to 30 percent!!!
static struct DACinterface     sns_dacInt = { 1, { 20 }};
static struct DACinterface     sns2_dacInt = { 2, { 20, 20 }};
static struct AY8910interface  sns_ay8910Int = { 1, 3579545./4., {25}, {sns_8910a_r}, {0}, {0}, {sns_8910b_w}};
// sns2_ay8910Int: There is no handler for the B port write of the 2nd AY chip. In fact the code never actually uses the additional functions, so the 2nd AY chip is never accessed on any (known) game/ROM that has the 1B13136 sound board.
static struct AY8910interface  sns2_ay8910Int = { 2, 3579545./4., {25, 25}, {sns_8910a_r, sns2_8910a_r}, {0, 0}, {0, 0}, {sns_8910b_w, 0}};

static READ_HANDLER(m00df_r) {
  return 0xff; // unmapped memory should usually read as all high bits
}

static MEMORY_READ_START(sns_readmem)
  { 0x0000, 0x007f, MRA_RAM },
  { 0x0080, 0x0083, pia_r(SNS_PIA0) },
  { 0x0084, 0x0087, pia_r(SNS_PIA2) }, // 13136 only
  { 0x0090, 0x0093, pia_r(SNS_PIA1) },
  { 0x00df, 0x00df, m00df_r }, // code flaw in farfalla @$FD7A: anda $00DF (94 opcode) instead of anda #$DF (84 opcode); bad dump maybe?!
  { 0x1800, 0x1800, sns2_8910a_r }, // 13136 only
  { 0x2000, 0x2000, sns_8910a_r },
  { 0x7000, 0xffff, MRA_ROM },
MEMORY_END

static MEMORY_WRITE_START(sns_writemem)
  { 0x0000, 0x007f, MWA_RAM },
  { 0x0080, 0x0083, pia_w(SNS_PIA0) },
  { 0x0084, 0x0087, pia_w(SNS_PIA2) }, // 13136 only
  { 0x0090, 0x0093, pia_w(SNS_PIA1) },
  { 0x1000, 0x1000, sns_dac_w },
  { 0x4000, 0x4000, DAC_1_data_w }, // 13136 only (never accessed)
MEMORY_END

static MACHINE_DRIVER_START(zac1370_nosound)
  MDRV_CPU_ADD(M6802, 3579545./4.)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(sns_readmem, sns_writemem)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(zac1370)
  MDRV_IMPORT_FROM(zac1370_nosound)

  MDRV_INTERLEAVE(500)
  MDRV_SOUND_ADD(TMS5220, sns_tms5220Int)
  MDRV_SOUND_ADD_TAG("ay8910", AY8910, sns_ay8910Int)
  MDRV_SOUND_ADD(DAC,     sns_dacInt)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(zac13136)
  MDRV_IMPORT_FROM(zac1370_nosound)

  MDRV_INTERLEAVE(500)
  MDRV_SOUND_ADD(TMS5220, sns_tms5220Int)
  MDRV_SOUND_ADD(AY8910,  sns2_ay8910Int)
  MDRV_SOUND_ADD(DAC,     sns2_dacInt)
MACHINE_DRIVER_END

static READ_HANDLER(sns_pia0a_r);
static WRITE_HANDLER(sns_pia0a_w);
static WRITE_HANDLER(sns_pia0b_w);
static WRITE_HANDLER(sns_pia0ca2_w);
static WRITE_HANDLER(sns_pia0cb2_w);
static READ_HANDLER(sns_pia1a_r);
static WRITE_HANDLER(sns_pia1a_w);
static WRITE_HANDLER(sns_pia1b_w);
static READ_HANDLER(sns_pia1ca2_r);
static READ_HANDLER(sns_pia2a_r);
static WRITE_HANDLER(sns_pia2a_w);
static WRITE_HANDLER(sns_pia2b_w);
static WRITE_HANDLER(sns_pia2ca2_w);

static void sns_irq0a(int state);
static void sns_irq0b(int state);
static void sns_irq1a(int state);
static void sns_irq1b(int state);

static void startcem3374(int param);
static void stopcem3374(int param);

static struct {
  struct sndbrdData brdData;
  int pia0a, pia0b, pia1a, pia1ca2, pia2a, pia2b;
  UINT8 lastcmd, daclatch, dacbyte1, dacbyte2;
  int dacMute,dacinp,channel;
  UINT8 snot_ab1, snot_ab2, snot_ab3, snot_ab4; // output from ls139 2d
  UINT8 s_ensynca,s_ensawb,s_entrigb,s_enpwma,s_ensawa,s_entriga,s_refsel,s_dacsh; // output from ls259 3h
  UINT8 s_inh4,s_inh3,s_inh2,s_inh1,s_envca,s_ensyncb; // output from ls259 3I
  int vcrfreq,rescntl,levchb,pwmb,freqb,levcha,pwma,freqa;
  int tmsPitch;
  mame_timer *fadeTimer;
  int vola, volb;
  UINT16 vcagain;
  int actflags;

  UINT8 old[75];
  int oldact;
  UINT8 cb1;
  UINT8 ignoreNext;
  UINT8 lastinh1;
  UINT8 lastAy;
  UINT8 timerSet;
  UINT8 solcmd;
} snslocals;

static const struct pia6821_interface sns_pia[] = {{
  /*i: A/B,CA/B1,CA/B2 */ sns_pia0a_r, 0, PIA_UNUSED_VAL(1), PIA_UNUSED_VAL(1), PIA_UNUSED_VAL(0), PIA_UNUSED_VAL(0),
  /*o: A/B,CA/B2       */ sns_pia0a_w, sns_pia0b_w, sns_pia0ca2_w, sns_pia0cb2_w,
  /*irq: A/B           */ sns_irq0a, sns_irq0b
},{
  /*i: A/B,CA/B1,CA/B2 */ sns_pia1a_r, 0, PIA_UNUSED_VAL(0), 0, sns_pia1ca2_r, PIA_UNUSED_VAL(0),
  /*o: A/B,CA/B2       */ sns_pia1a_w, sns_pia1b_w, 0, 0,
  /*irq: A/B           */ sns_irq1a, sns_irq1b
},{
  /*i: A/B,CA/B1,CA/B2 */ sns_pia2a_r, 0, PIA_UNUSED_VAL(1), PIA_UNUSED_VAL(1), PIA_UNUSED_VAL(0), PIA_UNUSED_VAL(0),
  /*o: A/B,CA/B2       */ sns_pia2a_w, sns_pia2b_w, sns_pia2ca2_w, 0,
  /*irq: A/B           */ 0, 0
}};

static void fade_timer(int param) {
  int dec = snslocals.rescntl / 128;
  if (!dec) dec = 16; // fade veeery slowly on 0 value :)
  else if (dec < 25) dec = 25; // slowest sensible fading speed, apart from 0
  if (snslocals.vola > dec) {
    snslocals.vola -= dec;
  } else {
    snslocals.vola = 0;
  }
  mixer_set_volume(snslocals.channel,   snslocals.vola * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
  mixer_set_volume(snslocals.channel+1, snslocals.vola * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
  if (snslocals.volb > dec) {
    snslocals.volb -= dec;
  } else {
    snslocals.volb = 0;
  }
  mixer_set_volume(snslocals.channel+2, snslocals.volb * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
  mixer_set_volume(snslocals.channel+3, snslocals.volb * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
}

static void initTMS(void) {
  tms5220_reset();
  if (!(core_gameData->hw.gameSpecific2 & USES_TMS_5220)) {
    // Pinball Champ ('82), Soccer Kings, Time Machine, Pool Champion, Black Belt, Mexico 86, Zankor (TMS5200)
    tms5220_set_variant(TMS5220_IS_5200);
  } else {
    // Farfalla, Devil Riders, Magic Castle, Robot, Zankor (TMS5220), Spooky, (New) Star's Phoenix, Thunder Man
    tms5220_set_variant(TMS5220_IS_5220);
  }
}

static void sns_init(struct sndbrdData *brdData) {
  int oldCh = snslocals.channel;
  if (snslocals.fadeTimer) timer_remove(snslocals.fadeTimer);
  memset(&snslocals, 0, sizeof(snslocals));
  snslocals.channel = oldCh;
  snslocals.brdData = *brdData;

  if (!(core_gameData->hw.soundBoard & 0x02)) {
    pia_config(SNS_PIA0, PIA_STANDARD_ORDERING, &sns_pia[0]);
  }
  pia_config(SNS_PIA1, PIA_STANDARD_ORDERING, &sns_pia[1]);
  snslocals.pia1a = 0xff;
  if (!(core_gameData->hw.soundBoard & 0x02)) {
    pia_config(SNS_PIA2, PIA_STANDARD_ORDERING, &sns_pia[2]);
  }
  // reset tms5220
  snslocals.tmsPitch = -1;
  snslocals.actflags = 3;
  initTMS();
  UpdateZACSoundACT(snslocals.actflags);

  if (core_gameData->hw.soundBoard & 0x02) {
    snslocals.fadeTimer = timer_alloc(fade_timer);
    timer_adjust(snslocals.fadeTimer, TIME_IN_USEC(12500), 0, TIME_IN_USEC(12500));
    snslocals.vcagain = 0xfff;
  }
}

static void sns_diag(int button) {
  if (core_gameData->hw.soundBoard & 0x02) return; // ignore 11178 - too bad subboards can't have their own sndbrdIntf!
  cpu_set_nmi_line(ZACSND_CPUA, button ? ASSERT_LINE : CLEAR_LINE);
}

static WRITE_HANDLER(sns_dac_w) {
  if (core_gameData->hw.soundBoard != SNDBRD_ZAC1370 || !snslocals.dacMute) DAC_0_data_w(offset, data);
}

static void writeToCEM(void) {
  int changed = 0;
  switch (pia_1_portb_r(0) >> 5) {
    case 0:
      changed = (snslocals.freqa != snslocals.dacinp);
      snslocals.freqa = snslocals.dacinp;
      break;
    case 1:
      changed = (snslocals.pwma != snslocals.dacinp);
      snslocals.pwma = snslocals.dacinp;
      break;
    case 2:
      snslocals.levcha = snslocals.dacinp;
      break;
    case 3:
      changed = (snslocals.freqb != snslocals.dacinp);
      snslocals.freqb = snslocals.dacinp;
      break;
    case 4:
      changed = (snslocals.pwmb != snslocals.dacinp);
      snslocals.pwmb = snslocals.dacinp;
      break;
    case 5:
      snslocals.levchb = snslocals.dacinp;
      break;
    case 6:
      snslocals.rescntl = snslocals.dacinp;
      break;
    case 7:
      changed = (snslocals.vcrfreq != snslocals.dacinp);
      if (changed && snslocals.vola < snslocals.levcha - 12) snslocals.vola += 12;
      if (changed && snslocals.volb < snslocals.levchb - 12) snslocals.volb += 12;
      snslocals.vcrfreq = snslocals.dacinp;
  }
  if (changed) startcem3374(snslocals.s_ensyncb);
}

static READ_HANDLER(sns_pia0a_r) {
  if (core_gameData->hw.gameSpecific2 & NO_AY8910) return 0xff;
  if ((snslocals.pia0b & 0x03) == 0x01) return AY8910Read(0);
  return 0;
}
static WRITE_HANDLER(sns_pia0a_w) {
  if (core_gameData->hw.gameSpecific2 & NO_AY8910) return;
  snslocals.pia0a = data;
  if (snslocals.pia0b & 0x02) AY8910Write(0, snslocals.pia0b ^ 0x01, snslocals.pia0a);
}
static WRITE_HANDLER(sns_pia0b_w) {
  if (core_gameData->hw.gameSpecific2 & NO_AY8910) return;
  if ((data & 0xf0) != (snslocals.pia0b & 0xf0)) logerror("DAC1408 modulation: %x\n", data >> 4);
  snslocals.pia0b = data;
  if (snslocals.pia0b & 0x02) AY8910Write(0, snslocals.pia0b ^ 0x01, snslocals.pia0a);
}
static WRITE_HANDLER(sns_pia0ca2_w) {
  UpdateZACSoundLED(1, data);
} // diag led
static WRITE_HANDLER(sns_pia0cb2_w) {
  snslocals.dacMute = data;
} // mute DAC

static READ_HANDLER(sns_pia1a_r) {
//  logerror("%04x:sns_pia1a_r %02x\n", activecpu_get_previouspc(),snslocals.pia1a);
  return snslocals.pia1a;
}
static WRITE_HANDLER(sns_pia1a_w) {
  logerror("%04x:sns_pia1a_w %02x\n", activecpu_get_previouspc(),data);
  snslocals.pia1a = data;
}
static WRITE_HANDLER(sns_pia1b_w) {
  logerror("%04x:sns_pia1b_w %02x\n", activecpu_get_previouspc(),data);

  if (pia_1_portb_r(0) & ~data & 0x01) { // read, overrides write command!
    snslocals.pia1a = tms5220_status_r(0);
  } else if (pia_1_portb_r(0) & ~data & 0x02) { // write
    tms5220_data_w(0, snslocals.pia1a);
  } else { // pull up port A if speech chip is not read from or written to, fixes garbled speech
    snslocals.pia1a = 0xff;
  }

  if (!(core_gameData->hw.soundBoard & 0x02) && (data & 0xf0) != (pia_1_portb_r(0) & 0xf0)) logerror("TMS5200 modulation: %x\n", data >> 4);
  pia_set_input_b(SNS_PIA1, data);
  pia_set_input_ca2(SNS_PIA1, tms5220_ready_r());

  if (core_gameData->hw.soundBoard == SNDBRD_ZAC11178) {
    if (snslocals.actflags != (data & 0x04 ? 0 : 3)) {
      snslocals.actflags = data & 0x04 ? 0 : 3; //both ACTSND & ACTSPK inverted on bit 2
      snslocals.vola = snslocals.volb = 0;
      UpdateZACSoundACT(snslocals.actflags);
    }
  } else if (core_gameData->hw.soundBoard == SNDBRD_ZAC11178_13181) {
    int old = snslocals.actflags;
    snslocals.actflags = (snslocals.actflags & 1) | (data & 0x04 ? 0 : 2); // ACTSND inverted on bit 2, ACTSPK from daughter board
    if (snslocals.actflags != old) {
      snslocals.vola = snslocals.volb = 0;
      UpdateZACSoundACT(snslocals.actflags);
    }
  } else {
    if (snslocals.actflags != ((data >> 2) & 0x03)) {
      snslocals.actflags = (data >> 2) & 0x03; //ACTSPK & ACTSND on bits 2 & 3
      UpdateZACSoundACT(snslocals.actflags);
    }
  }
  // set ab1 to ab4 switches based on pb3 & pb4 IC 2d
  if (core_gameData->hw.soundBoard & 0x02) { // true for all 11178
    snslocals.snot_ab4 = snslocals.snot_ab3 = snslocals.snot_ab2 = snslocals.snot_ab1 = 0;
    if ((data & 0x08) && (data & 0x10)) snslocals.snot_ab4 = SW_TRUE;   // pb3 and pb4 set -> snot_ab4 true
    if ((~data & 0x08) && (data & 0x10)) snslocals.snot_ab3 = SW_TRUE;
    if ((data & 0x08) && (~data & 0x10)) snslocals.snot_ab2 = SW_TRUE;
    if ((~data & 0x08) && (~data & 0x10)) snslocals.snot_ab1 = SW_TRUE;
//    logerror("sns_pia1b_w: data %x snot_ab4 %x snot_ab3 %x snot_ab2 %x snot_ab1 %x \n", data,snslocals.snot_ab4,snslocals.snot_ab3,snslocals.snot_ab2,snslocals.snot_ab1);
  }
}
static READ_HANDLER(sns_pia1ca2_r) {
//  logerror("sns_pia1ca2_r TMS5220 ready %x\n", snslocals.pia1ca2);
  return snslocals.pia1ca2;
}

static READ_HANDLER(sns_pia2a_r) {
  if ((snslocals.pia2b & 0x03) == 0x01) return AY8910Read(1);
  return 0;
}
static WRITE_HANDLER(sns_pia2a_w) {
  snslocals.pia2a = data;
  if (snslocals.pia2b & 0x02) AY8910Write(1, snslocals.pia2b ^ 0x01, snslocals.pia2a);
}
static WRITE_HANDLER(sns_pia2b_w) {
  snslocals.pia2b = data;
  if (snslocals.pia2b & 0x02) AY8910Write(1, snslocals.pia2b ^ 0x01, snslocals.pia2a);
}
static WRITE_HANDLER(sns_pia2ca2_w) {
  UpdateZACSoundLED(2, data);
} // diag led

static WRITE_HANDLER(sns_data_w) {
  snslocals.lastcmd = data;

  if ((core_gameData->hw.gameSpecific2 & NEEDS_TMS_RESET_ON_DATA) && (data & 0x80)) // some of the speech would be garbled otherwise!
    initTMS();

  switch (core_gameData->hw.soundBoard) {
    case SNDBRD_ZAC1370:
      pia_set_input_cb1(SNS_PIA0, data & 0x80 ? 1 : 0);
      break;
    case SNDBRD_ZAC13136:
      cpu_set_irq_line(ZACSND_CPUA, M6802_IRQ_LINE, data & 0x80 ? ASSERT_LINE : CLEAR_LINE);
      break;
    case SNDBRD_ZAC11178:
      pia_set_input_ca1(SNS_PIA1, data & 0x80 ? 1 : 0);
      break;
    case SNDBRD_ZAC11178_13181:
      pia_set_input_ca1(SNS_PIA1, (data & 0xc0) == 0xc0); // CA1 is fed by DB6 & DB7 from daughter board (same signals as on main board though)
      // cpu reads command from address b0 after nmi !!!
      if ((~data & 0x40) && (data & 0x80)) cpu_set_nmi_line(ZACSND_CPUB, PULSE_LINE);
      break;
    case SNDBRD_ZAC13181x3:
      if ((~data & 0x40) && (data & 0x80)) cpu_set_nmi_line(ZACSND_CPUB, PULSE_LINE);
      if ((data & 0x40) && (data & 0x80)) cpu_set_nmi_line(ZACSND_CPUC, PULSE_LINE);
  }
}

static READ_HANDLER(sns_8910a_r) { return ~snslocals.lastcmd; }

static WRITE_HANDLER(sns_8910b_w) {
  if ((snslocals.lastAy & 0x0f) != (data & 0x0f)) logerror("AY8910  modulation: %x\n", data & 0x0f);
  snslocals.lastAy = data;
}

static READ_HANDLER(sns2_8910a_r) { return ~snslocals.lastcmd; }

static void sns_irq0a(int state) {
//  logerror("sns_irq0a: state=%x\n",state);
  if (core_gameData->hw.soundBoard == SNDBRD_ZAC1370)
    cpu_set_irq_line(ZACSND_CPUA, M6802_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}
static void sns_irq0b(int state) {
//  logerror("sns_irq0b: state=%x\n",state);
  if (core_gameData->hw.soundBoard == SNDBRD_ZAC1370)
    cpu_set_irq_line(ZACSND_CPUA, M6802_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

static void fireNmi(int dummy) {
  if (snslocals.timerSet) {
    cpu_set_nmi_line(ZACSND_CPUA, PULSE_LINE);
  }
}

static void sns_irq1a(int state) {
//  logerror("%d:%04x:sns_irq1a: state=%x\n",cpu_getactivecpu(),activecpu_get_previouspc(),state);
  if (core_gameData->hw.soundBoard == SNDBRD_ZAC1370)
    cpu_set_irq_line(ZACSND_CPUA, M6802_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
  if (core_gameData->hw.soundBoard & 0x02) { // true for 11178
  	if (state) { // IRQA is set to 1 and back to 0 within 6 CPU cycles, but the NMI line assertion must be delayed slightly and must be cancelable
      snslocals.timerSet = 1;
      timer_set(TIME_IN_CYCLES(5, ZACSND_CPUA), 0, fireNmi);
    } else {
      snslocals.timerSet = 0;
    }
  }
}
static void sns_irq1b(int state) {
//  logerror("%d:%04x:sns_irq1b: state=%x\n",cpu_getactivecpu(),activecpu_get_previouspc(),state);
  if (core_gameData->hw.soundBoard == SNDBRD_ZAC1370 || core_gameData->hw.soundBoard & 0x02)
    cpu_set_irq_line(ZACSND_CPUA, M6802_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

static void sns_5220Irq(int state) {
  if ((core_getDip(0) >> 4) != snslocals.tmsPitch)
    tms5220_set_frequency((93 + (snslocals.tmsPitch = (core_getDip(0) >> 4))) * 6666.66666666666); // 0:620kHz .. 15:720kHz
  if (core_gameData->hw.soundBoard == SNDBRD_ZAC1370 || core_gameData->hw.soundBoard == SNDBRD_ZAC13136)
    pia_set_input_cb1(SNS_PIA1, !state);
//  logerror("sns_5220Irq: state=%x\n",state);
}
static void sns_5220Rdy(int state) {
  if (core_gameData->hw.soundBoard == SNDBRD_ZAC1370 || core_gameData->hw.soundBoard == SNDBRD_ZAC13136 || core_gameData->hw.soundBoard & 0x02)
    pia_set_input_ca2(SNS_PIA1, (snslocals.pia1ca2 = state));
//  logerror("sns_5220Rdy: state=%x\n",state);
}

/*----------------------------------------
/ Sound & Speech board 1B11178.
/ Uses only one PIA controlling the TMS chip and the rest of the board,
/ also carries a single DAC and some analog VCO components (CEM 3372/3374)
/ to replace the AY8910 chip.
/-----------------------------------------*/
// This is the E signal, divided by a 4040 chip's Q11, so 3.58 MHz / 4 / 2048, so ~437 Hz
#define SNS_11178_IRQFREQ (3579545./8192.)

static INTERRUPT_GEN(sns3_irq);
static WRITE_HANDLER(dacxfer);
static WRITE_HANDLER(storelatch);
static WRITE_HANDLER(storebyte1);
static WRITE_HANDLER(storebyte2);
static READ_HANDLER(readlatch);
static READ_HANDLER(readcmd);
static WRITE_HANDLER(chip3h259);
static WRITE_HANDLER(chip3i259);

static const UINT8 triangleWave[] = {
  0x00, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37, //    *
  0x3f, 0x47, 0x4f, 0x57, 0x5f, 0x67, 0x6f, 0x77, //   * *
  0x7f, 0x77, 0x6f, 0x67, 0x5f, 0x57, 0x4f, 0x47, //  *   *
  0x3f, 0x37, 0x2f, 0x27, 0x1f, 0x17, 0x0f, 0x07, // *     *
  0x00, 0xf9, 0xf1, 0xe9, 0xe1, 0xd9, 0xd1, 0xc9, //        *     *
  0xc1, 0xb9, 0xb1, 0xa9, 0xa1, 0x99, 0x91, 0x89, //         *   *
  0x81, 0x89, 0x91, 0x99, 0xa1, 0xa9, 0xb1, 0xb9, //          * *
  0xc1, 0xc9, 0xd1, 0xd9, 0xe1, 0xe9, 0xf1, 0xf9  //           *
};
static UINT8 triangleWaver[64];
static UINT8 triangleWave45[64];

static const UINT8 sawtoothWave[] = {
  0x02, 0x06, 0x0a, 0x0e, 0x12, 0x16, 0x1a, 0x1e, //       *
  0x22, 0x26, 0x2a, 0x2e, 0x32, 0x36, 0x3a, 0x3e, //     * *
  0x42, 0x46, 0x4a, 0x4e, 0x52, 0x57, 0x5b, 0x5f, //   *   *
  0x63, 0x67, 0x6b, 0x6f, 0x73, 0x77, 0x7b, 0x7f, // *     *
  0x81, 0x85, 0x89, 0x8d, 0x91, 0x95, 0x99, 0x9d, //       *     *
  0xa1, 0xa5, 0xa9, 0xae, 0xb2, 0xb6, 0xba, 0xbe, //       *   *
  0xc2, 0xc6, 0xca, 0xce, 0xd2, 0xd6, 0xda, 0xde, //       * *
  0xe2, 0xe6, 0xea, 0xee, 0xf2, 0xf6, 0xfa, 0xfe  //       *
};
static UINT8 sawtoothWaver[64];
static UINT8 sawtoothWave45[64];

static int sns_sh_start(const struct MachineSound *msound) {
  UINT8 i;
  for (i=0; i < 64; i++) {  // reverse waves
    triangleWaver[63-i]=triangleWave[i];
    sawtoothWaver[63-i]=sawtoothWave[i];
  }
  // apply some sort of 45 deg filter
  for (i = 0; i < 56; i++) {
    triangleWave45[i + 8] = triangleWave[i];
    sawtoothWave45[i + 8] = sawtoothWave[i];
  }
  for (i = 56; i < 64; i++) {
    triangleWave45[i - 56] = triangleWave[i];
    sawtoothWave45[i - 56] = sawtoothWave[i];
  }
  UpdateZACSoundLED(1, 1);
  if (!snslocals.channel) {
    // allocate channels
    static const int mixing_levels[4] = {MIXER(15,MIXER_PAN_LEFT),MIXER(15,MIXER_PAN_LEFT),MIXER(15,MIXER_PAN_RIGHT),MIXER(15,MIXER_PAN_RIGHT)};
    snslocals.channel = mixer_allocate_channels(4, mixing_levels);
    mixer_set_name  (snslocals.channel,   "CEM 3374 A TR");
    mixer_set_name  (snslocals.channel+1, "CEM 3374 A SA");
    mixer_set_name  (snslocals.channel+2, "CEM 3374 B TR");
    mixer_set_name  (snslocals.channel+3, "CEM 3374 B SA");
  }
  return 0;
}

struct CustomSound_interface sns_custInt = { sns_sh_start };

static MEMORY_READ_START(sns3_readmem)
  { 0x0000, 0x007f, MRA_RAM },
  { 0x0090, 0x0093, pia_r(SNS_PIA1) },
  { 0x00b0, 0x00bf, readcmd},
  { 0x00f0, 0x00ff, readlatch },
  { 0x5000, 0x5000, MRA_NOP },
  { 0x8000, 0xffff, MRA_ROM },
MEMORY_END

// TODO: remove this hack once we find out how the end of sounds are actually triggered
static WRITE_HANDLER(hack) {
  int i, allOnes = 1;
  memory_region(REGION_CPU2)[0x005e] = data;
  if (snslocals.actflags != snslocals.oldact) {
    memset(&snslocals.old, 0, sizeof(snslocals.old));
    snslocals.oldact = snslocals.actflags;
    return;
  }
  for (i = 0; i < sizeof(snslocals.old); i++) {
    if (snslocals.old[i] != 0x01) {
      allOnes = 0;
      break;
    }
  }
  if (allOnes) {
    memory_region(REGION_CPU2)[0x0017] = 0x10; // this will ultimately write to PIA port B bit 2
    memset(&snslocals.old, 0, sizeof(snslocals.old));
    return;
  }
  if (data != 0x01) {
    memset(&snslocals.old, 0, sizeof(snslocals.old));
  } else {
    for (i = sizeof(snslocals.old)-2; i >= 0; i--) {
      snslocals.old[i + 1] = snslocals.old[i];
    }
    snslocals.old[0] = data;
  }
}

static MEMORY_WRITE_START(sns3_writemem)
  { 0x005e, 0x005e, hack },
  { 0x0000, 0x007f, MWA_RAM },
  { 0x0080, 0x0087, chip3h259 },
  { 0x0090, 0x0093, pia_w(SNS_PIA1) },
  { 0x00a0, 0x00a7, chip3i259 },
  { 0x00c0, 0x00cf, dacxfer },
  { 0x00d0, 0x00df, storebyte1 },
  { 0x00e0, 0x00ef, storebyte2 },
  { 0x00f0, 0x00ff, storelatch },
MEMORY_END

MACHINE_DRIVER_START(zac11178)
  MDRV_CPU_ADD(M6802, 3579545./4.)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(sns3_readmem, sns3_writemem)
  MDRV_CPU_PERIODIC_INT(sns3_irq, SNS_11178_IRQFREQ)

  MDRV_INTERLEAVE(500)
  MDRV_SOUND_ADD(TMS5220, sns_tms5220Int)
  MDRV_SOUND_ADD(CUSTOM,  sns_custInt)
//  MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
MACHINE_DRIVER_END

static void stopcem3374(int param) {
  if (mixer_is_sample_playing(snslocals.channel))
    mixer_stop_sample(snslocals.channel);
  if (mixer_is_sample_playing(snslocals.channel+1))
    mixer_stop_sample(snslocals.channel+1);
  if (mixer_is_sample_playing(snslocals.channel+2))
    mixer_stop_sample(snslocals.channel+2);
  if (mixer_is_sample_playing(snslocals.channel+3))
    mixer_stop_sample(snslocals.channel+3);
}

static void startcem3374(int param) {
  signed char *triWave = param ? (signed char *)triangleWaver : (signed char *)triangleWave;
  signed char *sawWave = param ? (signed char *)sawtoothWaver : (signed char *)sawtoothWave;
  int triSize = param ? sizeof(triangleWaver) : sizeof(triangleWave);
  int sawSize = param ? sizeof(sawtoothWaver) : sizeof(sawtoothWave);

  int freqa = snslocals.s_entriga || snslocals.s_ensawa || snslocals.pwma || snslocals.s_ensyncb ? ((snslocals.s_enpwma ? snslocals.pwma : 0) + snslocals.freqa * snslocals.freqa) / 125 + snslocals.vcrfreq / 2 : 0;
  int freqb = snslocals.s_entrigb || snslocals.s_ensawb || snslocals.pwmb || snslocals.s_ensynca ? (snslocals.pwmb + snslocals.freqb * snslocals.freqb) / 125 + snslocals.vcrfreq / 2 : 0;

  if (snslocals.s_entriga || snslocals.s_ensyncb) {
    if (mixer_is_sample_playing(snslocals.channel))
      mixer_set_sample_frequency(snslocals.channel, snslocals.s_ensyncb ? (freqa + freqb) / 2 : freqa);
    else
      mixer_play_sample(snslocals.channel, (signed char *)triangleWave45, sizeof(triangleWave45), snslocals.s_ensyncb ? (freqa + freqb) / 2 : freqa, 1);
  } else
    mixer_stop_sample(snslocals.channel);

  if (snslocals.s_ensawa || snslocals.s_enpwma || snslocals.s_ensyncb) {
    if (mixer_is_sample_playing(snslocals.channel+1))
      mixer_set_sample_frequency(snslocals.channel+1, snslocals.s_ensyncb ? (freqa + freqb) / 2 : freqa);
    else
      mixer_play_sample(snslocals.channel+1, (signed char *)sawtoothWave45, sizeof(sawtoothWave45), snslocals.s_ensyncb ? (freqa + freqb) / 2 : freqa, 1);
  } else
    mixer_stop_sample(snslocals.channel+1);

  if (snslocals.s_entrigb || snslocals.s_ensynca) {
    if (mixer_is_sample_playing(snslocals.channel+2))
      mixer_set_sample_frequency(snslocals.channel+2, snslocals.s_ensynca ? (freqa + freqb) / 2 : freqb);
    else
      mixer_play_sample(snslocals.channel+2, triWave, triSize, snslocals.s_ensynca ? (freqa + freqb) / 2 : freqb, 1);
  } else
    mixer_stop_sample(snslocals.channel+2);

  if (snslocals.s_ensawb || snslocals.pwmb || snslocals.s_ensynca) {
    if (mixer_is_sample_playing(snslocals.channel+3))
      mixer_set_sample_frequency(snslocals.channel+3, snslocals.s_ensynca ? (freqa + freqb) / 2 : freqb);
    else
      mixer_play_sample(snslocals.channel+3, sawWave, sawSize, snslocals.s_ensynca ? (freqa + freqb) / 2 : freqb, 1);
  } else
    mixer_stop_sample(snslocals.channel+3);
//printf("FREQA:%03x PWMA:%03x LEVA:%03x FREQB:%03x PWMB:%03x LEVB:%03x RESCTRL:%03x VCRFREQ:%03x GAIN:%03x ", snslocals.freqa, snslocals.pwma, snslocals.levcha, snslocals.freqb, snslocals.pwmb, snslocals.levchb, snslocals.rescntl, snslocals.vcrfreq, snslocals.vcagain);
//printf("DACSH:%x REFSEL:%x TRIGA:%x SAWA:%x PWMA:%x TRIGB:%x SAWB:%x SYNCA:%x SYNCB:%x ", snslocals.s_dacsh, snslocals.s_refsel, snslocals.s_entriga, snslocals.s_ensawa, snslocals.s_enpwma, snslocals.s_entrigb, snslocals.s_ensawb, snslocals.s_ensynca, snslocals.s_ensyncb);
//printf("INH:%x%x%x%x\n", snslocals.s_inh1, snslocals.s_inh2, snslocals.s_inh3, snslocals.s_inh4);
}

static INTERRUPT_GEN(sns3_irq) {
  pia_set_input_cb1(SNS_PIA1, snslocals.cb1 = !snslocals.cb1);
#ifdef MAME_DEBUG
  if (keyboard_pressed_memory_repeat(KEYCODE_B, 30)) memory_region(REGION_CPU2)[0x0050] &= 0xfb;
  if (keyboard_pressed_memory_repeat(KEYCODE_N, 30)) memory_region(REGION_CPU2)[0x0037] = 0x10;
  if (keyboard_pressed_memory_repeat(KEYCODE_M, 30)) memory_region(REGION_CPU2)[0x0017] = 0x10;
#endif
}

// OK: the following addresses are only used by the 11178 sound board variants
static READ_HANDLER(readcmd) {
  UINT8 cmd = snslocals.lastcmd ^ 0xff;
  logerror("%04x:readcmd: %02x\n",activecpu_get_previouspc(), cmd);
  return cmd;
}
static READ_HANDLER(readlatch) {
  return snslocals.daclatch;
}
static WRITE_HANDLER(storelatch) {
  snslocals.daclatch = data;
}
static WRITE_HANDLER(storebyte1) {
  snslocals.dacbyte1 = snslocals.daclatch;
}
static WRITE_HANDLER(storebyte2) {
  snslocals.dacbyte2 = snslocals.daclatch;
}
static WRITE_HANDLER(dacxfer) {
// this dac uses 12 bits, so a 16 bit dac must be used...
// ok dac input: first byte (msb bit) second byte (only 4 bits are used and contains lsb bit)
  snslocals.dacinp = (snslocals.dacbyte1 << 4) | (snslocals.dacbyte2 >> 4);
// there is an extra DAC write between registers 0 and 7 used to feed VCAGAIN
  if (snslocals.ignoreNext) {
      snslocals.ignoreNext = 0;
    if (!snslocals.s_inh1 || snslocals.s_inh2 || snslocals.s_inh3) {
      stopcem3374(0);
    }
    return;
  }
  if (pia_1_portb_r(0) >> 5 == 0) snslocals.ignoreNext = 1;
  writeToCEM();
}

static WRITE_HANDLER(chip3h259) {
  int changed = 0;
  int flag = (data | ~pia_1_portb_r(0)) & 1;
  if (snslocals.snot_ab1) {     // Enable is logic low, latch adressable
    switch (offset) {
      case 0:
        changed = (snslocals.s_dacsh != flag);
        snslocals.s_dacsh = flag;
        break;
      case 1:
        changed = (snslocals.s_refsel != flag);
        snslocals.s_refsel = flag;
        break;
      case 2:
        changed = (snslocals.s_entriga != flag);
        if (changed) snslocals.vola = snslocals.levcha;
        snslocals.s_entriga = flag;
        break;
      case 3:
        changed = (snslocals.s_ensawa != flag);
        if (changed) snslocals.vola = snslocals.levcha;
        snslocals.s_ensawa = flag;
        break;
      case 4:
        changed = (snslocals.s_enpwma != flag);
        if (changed) snslocals.vola = snslocals.levcha;
        snslocals.s_enpwma = flag;
        break;
      case 5:
        changed = (snslocals.s_entrigb != flag);
        if (changed) snslocals.volb = snslocals.levchb;
        snslocals.s_entrigb = flag;
        break;
      case 6:
        changed = (snslocals.s_ensawb != flag);
        if (changed) snslocals.volb = snslocals.levchb;
        snslocals.s_ensawb = flag;
        break;
      case 7:
        changed = (snslocals.s_ensynca != flag);
        if (changed) {
          snslocals.vola = snslocals.levcha;
          snslocals.volb = snslocals.levchb;
          stream_update(snslocals.channel, 0);
          stream_update(snslocals.channel+1, 0);
        }
        snslocals.s_ensynca = flag;
    }
    if (changed) startcem3374(snslocals.s_ensyncb);
//    logerror("%04x:chip3h259: offset %x data %x s_dacsh %x s_refsel %x s_entriga %x s_ensawa %x s_enpwma %x s_entrigb %x s_ensawb %x s_ensynca %x \n",activecpu_get_previouspc(),offset,data,snslocals.s_dacsh,snslocals.s_refsel,snslocals.s_entriga,snslocals.s_ensawa,snslocals.s_enpwma ,snslocals.s_entrigb,snslocals.s_ensawb,snslocals.s_ensynca);
  }
}

static WRITE_HANDLER(chip3i259) {
  const int flag = (data | ~pia_1_portb_r(0)) & 1;
  if (snslocals.snot_ab1) {     // Enable is logic low, latch addressable
    int changed = 0;
    switch (offset) {
      case 0:
        changed = (snslocals.s_ensyncb != flag);
        if (changed) {
          snslocals.vola = snslocals.levcha;
          snslocals.volb = snslocals.levchb;
          stream_update(snslocals.channel+2, 0);
          stream_update(snslocals.channel+3, 0);
        }
        snslocals.s_ensyncb = flag;
        break;
      case 1:
        if (!flag && !snslocals.s_envca) {
          changed = 1;
          snslocals.vola = snslocals.levcha;
          snslocals.volb = snslocals.levchb;
        }
        snslocals.s_envca = flag;
        if (flag && snslocals.vcagain != snslocals.dacinp) {
          snslocals.vcagain = snslocals.dacinp;
          mixer_set_volume(snslocals.channel,   snslocals.vola * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
          mixer_set_volume(snslocals.channel+1, snslocals.vola * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
          mixer_set_volume(snslocals.channel+2, snslocals.volb * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
          mixer_set_volume(snslocals.channel+3, snslocals.volb * snslocals.vcagain * 100u / (0xfffu * 0xfffu));
        }
        break;
      case 2:
        snslocals.s_inh1 = flag;
        if (flag && snslocals.lastinh1) {
          changed = 1;
          snslocals.vola = snslocals.levcha;
          snslocals.volb = snslocals.levchb;
        }
        snslocals.lastinh1 = flag;
        break;
      case 3:
        changed = (snslocals.s_inh2 != flag);
        snslocals.s_inh2 = flag;
        if (changed) {
          snslocals.vola = snslocals.levcha;
          snslocals.volb = snslocals.levchb;
        }
        break;
      case 4:
        changed = (snslocals.s_inh3 != flag);
        snslocals.s_inh3 = flag;
        if (changed) {
          snslocals.vola = snslocals.levcha;
          snslocals.volb = snslocals.levchb;
        }
        break;
      case 5:
        changed = (snslocals.s_inh4 != flag);
        if (changed) {
          snslocals.vola = snslocals.levcha;
          snslocals.volb = snslocals.levchb;
        }
        snslocals.s_inh4 = flag;
    }
    if (changed) startcem3374(snslocals.s_ensyncb);
//    logerror("%04x:chip3i259: offset %x data %x s_ensyncb %x s_envca %x s_inh1 %x s_inh2 %x s_inh3 %x s_inh4 %x \n",activecpu_get_previouspc(),offset,data,snslocals.s_ensyncb,snslocals.s_envca,snslocals.s_inh1,snslocals.s_inh2,snslocals.s_inh3 ,snslocals.s_inh4);
  }
}


/*----------------------------------------
/ Sound & Speech board 1B11178 with 1B13181 / 1B11181 daughter board.
/ This daughter board is equipped with a Z80 processor, and
/ plays background music (all done with DACs).
/ The earlier 1B13181/0 single-channel version was used on Zankor only,
/ Spooky already has the full three-channel board 1B11181/1.
/ And even though only one DAC serves all the channels,
/ in the emulation, three DACs are used for cleaner sound output.
/
/ Also Sound & Speech board 1B11183 with three Z80 CPUs, used on Star's Phoenix.
/ One CPU does the exact same stuff as on the 1B11181 daughter board,
/ the 2nd CPU handles speech and effects,
/ and the 3rd one acts as a slave to the first CPU to play a musical tune.
/ That's five DAC chips playing at the same time! :)
/-----------------------------------------*/
static struct DACinterface     z80_1dacInt = { 1, { 50 }};
static struct DACinterface     z80_3dacInt = { 3, { 15, 20, 20 }};
static struct DACinterface     z80_5dacInt = { 5, { 20, 30, 30, 20, 30 }};

static MEMORY_READ_START(z80_readmem)
  { 0x0000, 0xfbff, MRA_ROM },
  { 0xfc00, 0xffff, MRA_RAM },
MEMORY_END

static MEMORY_WRITE_START(z80_writemem)
  { 0xfc00, 0xffff, MWA_RAM },
MEMORY_END

static WRITE_HANDLER(DAC_2_signed_data_w) { DAC_signed_data_w(2, data); }
static WRITE_HANDLER(DAC_3_signed_data_w) { DAC_signed_data_w(3, data); }
static WRITE_HANDLER(DAC_4_signed_data_w) { DAC_signed_data_w(4, data); }

static WRITE_HANDLER(akl_w) {
  int old = snslocals.actflags;
  snslocals.actflags = (snslocals.actflags & 2) | (~data & 1);
  if (old != snslocals.actflags) {
    UpdateZACSoundACT(snslocals.actflags);
  }
}

static WRITE_HANDLER(akl1_w) {
  akl_w(offset, data);
  cpu_set_nmi_line(ZACSND_CPUA, PULSE_LINE);
}

static WRITE_HANDLER(akl2_w) {
  // unused
}

static WRITE_HANDLER(akl3_w) {
  int old = snslocals.actflags;
  snslocals.actflags = (snslocals.actflags & 1) | (~data & 1) << 1;
  if (old != snslocals.actflags) {
    UpdateZACSoundACT(snslocals.actflags);
  }
}

static WRITE_HANDLER(sol_w) {
  snslocals.solcmd = data;
}

static READ_HANDLER(readcmd_a) {
  return snslocals.solcmd;
}

static PORT_READ_START(z80_readport)
  { 0x01, 0x01, readcmd },
PORT_END

static PORT_READ_START(z80_readport_a)
  { 0x01, 0x01, readcmd_a },
PORT_END

static PORT_WRITE_START(z80_writeport_a)
  { 0x00, 0x00, DAC_4_signed_data_w },
  { 0x02, 0x02, akl2_w },
PORT_END

static PORT_WRITE_START(z80_writeport_b)
  { 0x00, 0x00, DAC_0_signed_data_w },
  { 0x02, 0x02, akl_w },
  { 0x03, 0x03, MWA_NOP },
  { 0x04, 0x04, DAC_1_signed_data_w },
  { 0x08, 0x08, DAC_2_signed_data_w },
PORT_END
static PORT_WRITE_START(z80_writeport_b2)
  { 0x00, 0x00, DAC_0_signed_data_w },
  { 0x02, 0x02, akl1_w },
  { 0x03, 0x03, sol_w },
  { 0x04, 0x04, DAC_1_signed_data_w },
  { 0x08, 0x08, DAC_2_signed_data_w },
PORT_END

static PORT_READ_START(z80_readport_c)
  { 0x01, 0x01, readcmd },
  { 0x03, 0x03, tms5220_status_r },
PORT_END
static PORT_WRITE_START(z80_writeport_c)
  { 0x00, 0x00, DAC_3_signed_data_w },
  { 0x02, 0x02, akl3_w },
  { 0x03, 0x03, tms5220_data_w },
PORT_END

static INTERRUPT_GEN(cpu_a_irq) { cpu_set_irq_line(ZACSND_CPUA, 0, PULSE_LINE); }
static INTERRUPT_GEN(cpu_b_irq) { cpu_set_irq_line(ZACSND_CPUB, 0, PULSE_LINE); }
static INTERRUPT_GEN(cpu_c_irq) { cpu_set_irq_line(ZACSND_CPUC, 0, PULSE_LINE); }

static MACHINE_DRIVER_START(zac11178_13181_nodac)
  MDRV_IMPORT_FROM(zac11178)

  MDRV_CPU_ADD(Z80, 4000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(z80_readmem, z80_writemem)
  MDRV_CPU_PORTS(z80_readport, z80_writeport_b)
  MDRV_CPU_PERIODIC_INT(cpu_b_irq, 4000000./65536.)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(zac11178_13181)
  MDRV_IMPORT_FROM(zac11178_13181_nodac)
  MDRV_SOUND_ADD(DAC,     z80_1dacInt)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(zac11178_11181)
  MDRV_IMPORT_FROM(zac11178_13181_nodac)
  MDRV_SOUND_ADD(DAC,     z80_3dacInt)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(zac11183)
  MDRV_CPU_ADD(Z80, 4096000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(z80_readmem, z80_writemem)
  MDRV_CPU_PORTS(z80_readport_a, z80_writeport_a)
  MDRV_CPU_PERIODIC_INT(cpu_a_irq, 4096000./65536.)

  MDRV_CPU_ADD(Z80, 4000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(z80_readmem, z80_writemem)
  MDRV_CPU_PORTS(z80_readport, z80_writeport_b2)
  MDRV_CPU_PERIODIC_INT(cpu_b_irq, 4000000./65536.)

  MDRV_CPU_ADD(Z80, 4000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(z80_readmem, z80_writemem)
  MDRV_CPU_PORTS(z80_readport_c, z80_writeport_c)
  MDRV_CPU_PERIODIC_INT(cpu_c_irq, 4000000./65536.)

  MDRV_INTERLEAVE(500)
  MDRV_SOUND_ADD(TMS5220, sns_tms5220Int)
  MDRV_SOUND_ADD(DAC,     z80_5dacInt)
MACHINE_DRIVER_END
