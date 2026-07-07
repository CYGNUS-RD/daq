/********************************************************************\

  Name:         dd_sy4527.h
  Created by:   based on null.h / Stefan Ritt

  Contents:     Device driver function declarations for SY4527 device

  $Id$

\********************************************************************/

#ifndef CMD_GET_CURRENT_DET
#define CMD_GET_CURRENT_DET 50000
#endif

#ifndef CMD_CLEAR_ALARM
#define CMD_CLEAR_ALARM 50001
#endif

INT dd_sy4527(INT cmd, ...);
