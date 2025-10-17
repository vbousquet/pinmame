// license:BSD-3-Clause
// copyright-holders:Carsten W�chter
#pragma once

#ifdef __cplusplus
#if _MSC_VER >= 1700
 #ifdef inline
  #undef inline
 #endif
#endif
#endif

// Std Library includes
#include <stdio.h>

// Local includes
#include "../../ext/bass/bass.h"

#ifdef __cplusplus
  extern "C" {
#endif
  #include "driver.h"
  #include <dirent.h>
#ifdef __cplusplus
  }
#endif

#define ALT_MAX_CMDS 4

// ---------------------------------------------------------------------------
// Data Structures
// ---------------------------------------------------------------------------

	  // Structure for command data
	  typedef struct _cmd_data {
		  unsigned int cmd_counter;
		  int stored_command;
		  unsigned int cmd_filter;
		  unsigned int cmd_buffer[ALT_MAX_CMDS];
	  } CmdData;

// ---------------------------------------------------------------------------
// snd_alt function prototypes
// ---------------------------------------------------------------------------

#ifdef __cplusplus
  extern "C" {
#endif
  // Main entrypoint for AltSound handling
  void alt_sound_handle(int boardNo, int cmd);

  // Pause/Resume all streams
  void alt_sound_pause(BOOL pause);

  // Exit AltSound processing and clean up
  void alt_sound_exit();
#ifdef __cplusplus
  }
#endif

// ---------------------------------------------------------------------------
// Helper function prototypes
// ---------------------------------------------------------------------------

// Function to initialize all AltSound variables and data structures
BOOL alt_sound_init(CmdData* cmds_out);

// Function to pre-process commands based on ROM hardware platform
void preprocess_commands(CmdData* cmds_out, int cmd);

// Function to process combined commands based on ROM hardware platform
void postprocess_commands(const unsigned int combined_cmd);
