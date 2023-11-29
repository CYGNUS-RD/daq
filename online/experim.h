/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Wed Mar 16 11:51:10 2022

\********************************************************************/

#ifndef EXCL_TRIGGER

#define TRIGGER_COMMON_DEFINED

typedef struct {
  UINT16    event_id;
  UINT16    trigger_mask;
  char      buffer[32];
  INT32     type;
  INT32     source;
  char      format[8];
  BOOL      enabled;
  INT32     read_on;
  INT32     period;
  double    event_limit;
  UINT32    num_subevents;
  INT32     log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT32     write_cache_size;
} TRIGGER_COMMON;

#define TRIGGER_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = UINT16 : 1",\
"Trigger mask = UINT16 : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT32 : 2",\
"Source = INT32 : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT32 : 1",\
"Period = INT32 : 100",\
"Event limit = DOUBLE : 0",\
"Num subevents = UINT32 : 0",\
"Log history = INT32 : 0",\
"Frontend host = STRING : [32] localhost",\
"Frontend name = STRING : [32] cygnus_daq",\
"Frontend file name = STRING : [256] /home/standard/daq/online/cygnus_fe.cxx",\
"Status = STRING : [256] cygnus_daq@localhost",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT32 : 0",\
"",\
NULL }

#endif

#ifndef EXCL_CATHODE

#define CATHODE_COMMON_DEFINED

typedef struct {
  UINT16    event_id;
  UINT16    trigger_mask;
  char      buffer[32];
  INT32     type;
  INT32     source;
  char      format[8];
  BOOL      enabled;
  INT32     read_on;
  INT32     period;
  double    event_limit;
  UINT32    num_subevents;
  INT32     log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT32     write_cache_size;
} CATHODE_COMMON;

#define CATHODE_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = UINT16 : 4",\
"Trigger mask = UINT16 : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT32 : 16",\
"Source = INT32 : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT32 : 255",\
"Period = INT32 : 60000",\
"Event limit = DOUBLE : 0",\
"Num subevents = UINT32 : 0",\
"Log history = INT32 : 1",\
"Frontend host = STRING : [32] localhost",\
"Frontend name = STRING : [32] SC Frontend",\
"Frontend file name = STRING : [256] /home/standard/daq/online/scfe.cxx",\
"Status = STRING : [256] Ok",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT32 : 0",\
"",\
NULL }

#define CATHODE_SETTINGS_DEFINED

typedef struct {
  struct {
    struct {
      BOOL      enabled;
      struct {
        char      system_name[32];
        char      ip[32];
        INT32     port;
      } dd;
    } iseg_hps;
  } devices;
  char      editable[2][32];
  char      names[32];
  float     update_threshold_measured;
  float     update_threshold_current;
  float     zero_threshold;
  float     voltage_limit;
  float     current_limit;
  float     trip_time;
  float     ramp_up_speed;
  float     ramp_down_speed;
} CATHODE_SETTINGS;

#define CATHODE_SETTINGS_STR(_name) const char *_name[] = {\
"[Devices/iseg_hps]",\
"Enabled = BOOL : y",\
"",\
"[Devices/iseg_hps/DD]",\
"System Name = STRING : [32] iseg_hps",\
"IP = STRING : [32] 192.168.0.104",\
"Port = INT32 : 10001",\
"",\
"[.]",\
"Editable = STRING[2] :",\
"[32] Demand",\
"[32] ChState",\
"Names = STRING : [32] ISEG_HPS",\
"Update Threshold Measured = FLOAT : 0.01",\
"Update Threshold Current = FLOAT : 0.0001",\
"Zero Threshold = FLOAT : 20",\
"Voltage Limit = FLOAT : 45000",\
"Current Limit = FLOAT : 100",\
"Trip Time = FLOAT : 0",\
"Ramp Up Speed = FLOAT : 10000",\
"Ramp Down Speed = FLOAT : 10000",\
"",\
NULL }

#endif

#ifndef EXCL_GASSYSTEM

#define GASSYSTEM_COMMON_DEFINED

typedef struct {
  UINT16    event_id;
  UINT16    trigger_mask;
  char      buffer[32];
  INT32     type;
  INT32     source;
  char      format[8];
  BOOL      enabled;
  INT32     read_on;
  INT32     period;
  double    event_limit;
  UINT32    num_subevents;
  INT32     log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT32     write_cache_size;
} GASSYSTEM_COMMON;

#define GASSYSTEM_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = UINT16 : 6",\
"Trigger mask = UINT16 : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT32 : 16",\
"Source = INT32 : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT32 : 255",\
"Period = INT32 : 60000",\
"Event limit = DOUBLE : 0",\
"Num subevents = UINT32 : 0",\
"Log history = INT32 : 1",\
"Frontend host = STRING : [32] localhost",\
"Frontend name = STRING : [32] SC Frontend",\
"Frontend file name = STRING : [256] /home/standard/daq/online/scfe.cxx",\
"Status = STRING : [256] Ok",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT32 : 0",\
"",\
NULL }

#define GASSYSTEM_SETTINGS_DEFINED

typedef struct {
  struct {
    struct {
      char      system_name[32];
      char      ip[32];
      INT32     namespace_index;
      char      tags_guid[64];
    } gassystem;
  } devices;
  char      names[323][32];
  float     update_threshold_measured[323];
} GASSYSTEM_SETTINGS;

#define GASSYSTEM_SETTINGS_STR(_name) const char *_name[] = {\
"[Devices/gassystem]",\
"System Name = STRING : [32] gassys",\
"IP = STRING : [32] 192.168.0.105:4870",\
"Namespace Index = INT32 : 3",\
"Tags Guid = STRING : [64] ecef81d5-c834-4379-ab79-c8fa3133a311",\
"",\
"[.]",\
"Names = STRING[323] :",\
"[32] @DiagnosticsIndicato",\
"[32] DB_CycleSelection_MO",\
"[32] DB_CycleSelection_MO",\
"[32] DB_Logic_AUTO_SEQ_Ci",\
"[32] DB_Logic_AUTO_SEQ_Ci",\
"[32] DB_Logic_AUTO_SEQ_Ci",\
"[32] DB_Logic_GraphDateAn",\
"[32] DB_Logic_LocalDateAn",\
"[32] DB_Logic_Miscelaz_Ca",\
"[32] DB_Logic_Miscelaz_Ca",\
"[32] DB_Logic_Miscelaz_Ca",\
"[32] DB_Logic_Miscelazion",\
"[32] DB_Logic_PID_Camera_",\
"[32] DB_Logic_PID_D404_Ou",\
"[32] DB_Logic_Press_Camer",\
"[32] DB_Logic_StatoImpian",\
"[32] DB_PAR_Alarm_Reset",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT401",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_Analisi_AT402",\
"[32] DB_PAR_CMD_MAN_EV101",\
"[32] DB_PAR_CMD_MAN_EV201",\
"[32] DB_PAR_CMD_MAN_EV301",\
"[32] DB_PAR_CMD_MAN_EV401",\
"[32] DB_PAR_CMD_MAN_EV402",\
"[32] DB_PAR_CMD_MAN_EV405",\
"[32] DB_PAR_CMD_MAN_EV406",\
"[32] DB_PAR_CMD_MAN_EV410",\
"[32] DB_PAR_Camera_SET_SE",\
"[32] DB_PAR_Camera_SET_SE",\
"[32] DB_PAR_Cicalino_Abil",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT1",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT2",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT3",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_FCT4",\
"[32] DB_PAR_Flowrate_Reg_",\
"[32] DB_PAR_Flowrate_Reg_",\
"[32] DB_PAR_Flowrate_Reg_",\
"[32] DB_PAR_Flowrate_Reg_",\
"[32] DB_PAR_Flowrate_Reg_",\
"[32] DB_PAR_HMI_MAN_AUT",\
"[32] DB_PAR_HMI_PULS_STAR",\
"[32] DB_PAR_HMI_PULS_STOP",\
"[32] DB_PAR_Miscelaz_Rice",\
"[32] DB_PAR_Miscelaz_Rice",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_Miscelaz_SET_",\
"[32] DB_PAR_PID_Camera_Ma",\
"[32] DB_PAR_PID_Camera_Ou",\
"[32] DB_PAR_PID_Camera_Ou",\
"[32] DB_PAR_PID_Camera_Pa",\
"[32] DB_PAR_PID_Camera_Pa",\
"[32] DB_PAR_PID_Camera_Pa",\
"[32] DB_PAR_PID_Camera_SP",\
"[32] DB_PAR_PID_D404_Man_",\
"[32] DB_PAR_PID_D404_Outp",\
"[32] DB_PAR_PID_D404_Outp",\
"[32] DB_PAR_PID_D404_Para",\
"[32] DB_PAR_PID_D404_Para",\
"[32] DB_PAR_PID_D404_Para",\
"[32] DB_PAR_PID_D404_SP",\
"[32] DB_PAR_Press_Atm",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT10",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT20",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT30",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT40",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Pressure_PT_C",\
"[32] DB_PAR_Purificaz_SET",\
"[32] DB_PAR_Purificaz_SET",\
"[32] DB_PAR_Purificaz_SET",\
"[32] DB_PAR_Ricircolo_SET",\
"[32] DB_PAR_Ricircolo_SET",\
"[32] DB_PAR_Ricircolo_SET",\
"[32] DB_PAR_Ricircolo_SET",\
"[32] DB_PAR_Ricircolo_SET",\
"[32] DB_PAR_SP_Rit_Alarm_",\
"[32] DB_STS_AlarmActive",\
"[32] DB_STS_Analisi_AT401",\
"[32] DB_STS_Analisi_AT401",\
"[32] DB_STS_Analisi_AT402",\
"[32] DB_STS_Analisi_AT402",\
"[32] DB_STS_FCT401_Corr",\
"[32] DB_STS_FT_Tot",\
"[32] DB_STS_Flowrate_FCT1",\
"[32] DB_STS_Flowrate_FCT1",\
"[32] DB_STS_Flowrate_FCT2",\
"[32] DB_STS_Flowrate_FCT2",\
"[32] DB_STS_Flowrate_FCT3",\
"[32] DB_STS_Flowrate_FCT3",\
"[32] DB_STS_Flowrate_FCT4",\
"[32] DB_STS_Flowrate_FCT4",\
"[32] DB_STS_Pressure_PT10",\
"[32] DB_STS_Pressure_PT10",\
"[32] DB_STS_Pressure_PT10",\
"[32] DB_STS_Pressure_PT10",\
"[32] DB_STS_Pressure_PT20",\
"[32] DB_STS_Pressure_PT20",\
"[32] DB_STS_Pressure_PT20",\
"[32] DB_STS_Pressure_PT20",\
"[32] DB_STS_Pressure_PT30",\
"[32] DB_STS_Pressure_PT30",\
"[32] DB_STS_Pressure_PT30",\
"[32] DB_STS_Pressure_PT30",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Pressure_PT40",\
"[32] DB_STS_Valves_EV101_",\
"[32] DB_STS_Valves_EV201_",\
"[32] DB_STS_Valves_EV301_",\
"[32] DB_STS_Valves_EV401_",\
"[32] DB_STS_Valves_EV401_",\
"[32] DB_STS_Valves_EV402_",\
"[32] DB_STS_Valves_EV402_",\
"[32] DB_STS_Valves_EV405_",\
"[32] DB_STS_Valves_EV406_",\
"[32] DB_STS_Valves_EV410_",\
"[32] PT_Segnalazioni di g",\
"[32] VIS_POP_AT401",\
"[32] VIS_POP_AT402",\
"[32] VIS_POP_FCT101",\
"[32] VIS_POP_FCT101_MAN",\
"[32] VIS_POP_FCT201",\
"[32] VIS_POP_FCT201_MAN",\
"[32] VIS_POP_FCT301",\
"[32] VIS_POP_FCT301_MAN",\
"[32] VIS_POP_FCT401",\
"[32] VIS_POP_FCT401_MAN",\
"[32] VIS_POP_G401_MAN",\
"[32] VIS_POP_PCV401_MAN",\
"[32] VIS_POP_PT101",\
"[32] VIS_POP_PT102",\
"[32] VIS_POP_PT201",\
"[32] VIS_POP_PT202",\
"[32] VIS_POP_PT301",\
"[32] VIS_POP_PT302",\
"[32] VIS_POP_PT404",\
"[32] VIS_POP_PT405",\
"[32] VIS_POP_PT406",\
"[32] VIS_POP_PT_Camera",\
"[32] Variabile_Numero di ",\
"[32] iDB_Utilities_System",\
"Update Threshold Measured = FLOAT[323] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"[8] 1",\
"[9] 1",\
"[10] 1",\
"[11] 1",\
"[12] 1",\
"[13] 1",\
"[14] 1",\
"[15] 1",\
"[16] 1",\
"[17] 1",\
"[18] 1",\
"[19] 1",\
"[20] 1",\
"[21] 1",\
"[22] 1",\
"[23] 1",\
"[24] 1",\
"[25] 1",\
"[26] 1",\
"[27] 1",\
"[28] 1",\
"[29] 1",\
"[30] 1",\
"[31] 1",\
"[32] 1",\
"[33] 1",\
"[34] 1",\
"[35] 1",\
"[36] 1",\
"[37] 1",\
"[38] 1",\
"[39] 1",\
"[40] 1",\
"[41] 1",\
"[42] 1",\
"[43] 1",\
"[44] 1",\
"[45] 1",\
"[46] 1",\
"[47] 1",\
"[48] 1",\
"[49] 1",\
"[50] 1",\
"[51] 1",\
"[52] 1",\
"[53] 1",\
"[54] 1",\
"[55] 1",\
"[56] 1",\
"[57] 1",\
"[58] 1",\
"[59] 1",\
"[60] 1",\
"[61] 1",\
"[62] 1",\
"[63] 1",\
"[64] 1",\
"[65] 1",\
"[66] 1",\
"[67] 1",\
"[68] 1",\
"[69] 1",\
"[70] 1",\
"[71] 1",\
"[72] 1",\
"[73] 1",\
"[74] 1",\
"[75] 1",\
"[76] 1",\
"[77] 1",\
"[78] 1",\
"[79] 1",\
"[80] 1",\
"[81] 1",\
"[82] 1",\
"[83] 1",\
"[84] 1",\
"[85] 1",\
"[86] 1",\
"[87] 1",\
"[88] 1",\
"[89] 1",\
"[90] 1",\
"[91] 1",\
"[92] 1",\
"[93] 1",\
"[94] 1",\
"[95] 1",\
"[96] 1",\
"[97] 1",\
"[98] 1",\
"[99] 1",\
"[100] 1",\
"[101] 1",\
"[102] 1",\
"[103] 1",\
"[104] 1",\
"[105] 1",\
"[106] 1",\
"[107] 1",\
"[108] 1",\
"[109] 1",\
"[110] 1",\
"[111] 1",\
"[112] 1",\
"[113] 1",\
"[114] 1",\
"[115] 1",\
"[116] 1",\
"[117] 1",\
"[118] 1",\
"[119] 1",\
"[120] 1",\
"[121] 1",\
"[122] 1",\
"[123] 1",\
"[124] 1",\
"[125] 1",\
"[126] 1",\
"[127] 1",\
"[128] 1",\
"[129] 1",\
"[130] 1",\
"[131] 1",\
"[132] 1",\
"[133] 1",\
"[134] 1",\
"[135] 1",\
"[136] 1",\
"[137] 1",\
"[138] 1",\
"[139] 1",\
"[140] 1",\
"[141] 1",\
"[142] 1",\
"[143] 1",\
"[144] 1",\
"[145] 1",\
"[146] 1",\
"[147] 1",\
"[148] 1",\
"[149] 1",\
"[150] 1",\
"[151] 1",\
"[152] 1",\
"[153] 1",\
"[154] 1",\
"[155] 1",\
"[156] 1",\
"[157] 1",\
"[158] 1",\
"[159] 1",\
"[160] 1",\
"[161] 1",\
"[162] 1",\
"[163] 1",\
"[164] 1",\
"[165] 1",\
"[166] 1",\
"[167] 1",\
"[168] 1",\
"[169] 1",\
"[170] 1",\
"[171] 1",\
"[172] 1",\
"[173] 1",\
"[174] 1",\
"[175] 1",\
"[176] 1",\
"[177] 1",\
"[178] 1",\
"[179] 1",\
"[180] 1",\
"[181] 1",\
"[182] 1",\
"[183] 1",\
"[184] 1",\
"[185] 1",\
"[186] 1",\
"[187] 1",\
"[188] 1",\
"[189] 1",\
"[190] 1",\
"[191] 1",\
"[192] 1",\
"[193] 1",\
"[194] 1",\
"[195] 1",\
"[196] 1",\
"[197] 1",\
"[198] 1",\
"[199] 1",\
"[200] 1",\
"[201] 1",\
"[202] 1",\
"[203] 1",\
"[204] 1",\
"[205] 1",\
"[206] 1",\
"[207] 1",\
"[208] 1",\
"[209] 1",\
"[210] 1",\
"[211] 1",\
"[212] 1",\
"[213] 1",\
"[214] 1",\
"[215] 1",\
"[216] 1",\
"[217] 1",\
"[218] 1",\
"[219] 1",\
"[220] 1",\
"[221] 1",\
"[222] 1",\
"[223] 1",\
"[224] 1",\
"[225] 1",\
"[226] 1",\
"[227] 1",\
"[228] 1",\
"[229] 1",\
"[230] 1",\
"[231] 1",\
"[232] 1",\
"[233] 1",\
"[234] 1",\
"[235] 1",\
"[236] 1",\
"[237] 1",\
"[238] 1",\
"[239] 1",\
"[240] 1",\
"[241] 1",\
"[242] 1",\
"[243] 1",\
"[244] 1",\
"[245] 1",\
"[246] 1",\
"[247] 1",\
"[248] 1",\
"[249] 1",\
"[250] 1",\
"[251] 1",\
"[252] 1",\
"[253] 1",\
"[254] 1",\
"[255] 1",\
"[256] 1",\
"[257] 1",\
"[258] 1",\
"[259] 1",\
"[260] 1",\
"[261] 1",\
"[262] 1",\
"[263] 1",\
"[264] 1",\
"[265] 1",\
"[266] 1",\
"[267] 1",\
"[268] 1",\
"[269] 1",\
"[270] 1",\
"[271] 1",\
"[272] 1",\
"[273] 1",\
"[274] 1",\
"[275] 1",\
"[276] 1",\
"[277] 1",\
"[278] 1",\
"[279] 1",\
"[280] 1",\
"[281] 1",\
"[282] 1",\
"[283] 1",\
"[284] 1",\
"[285] 1",\
"[286] 1",\
"[287] 1",\
"[288] 1",\
"[289] 1",\
"[290] 1",\
"[291] 1",\
"[292] 1",\
"[293] 1",\
"[294] 1",\
"[295] 1",\
"[296] 1",\
"[297] 1",\
"[298] 1",\
"[299] 1",\
"[300] 1",\
"[301] 1",\
"[302] 1",\
"[303] 1",\
"[304] 1",\
"[305] 1",\
"[306] 1",\
"[307] 1",\
"[308] 1",\
"[309] 1",\
"[310] 1",\
"[311] 1",\
"[312] 1",\
"[313] 1",\
"[314] 1",\
"[315] 1",\
"[316] 1",\
"[317] 1",\
"[318] 1",\
"[319] 1",\
"[320] 1",\
"[321] 1",\
"[322] 1",\
"",\
NULL }

#endif

#ifndef EXCL_HV

#define HV_COMMON_DEFINED

typedef struct {
  UINT16    event_id;
  UINT16    trigger_mask;
  char      buffer[32];
  INT32     type;
  INT32     source;
  char      format[8];
  BOOL      enabled;
  INT32     read_on;
  INT32     period;
  double    event_limit;
  UINT32    num_subevents;
  INT32     log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT32     write_cache_size;
} HV_COMMON;

#define HV_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = UINT16 : 3",\
"Trigger mask = UINT16 : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT32 : 16",\
"Source = INT32 : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT32 : 255",\
"Period = INT32 : 60000",\
"Event limit = DOUBLE : 0",\
"Num subevents = UINT32 : 0",\
"Log history = INT32 : 1",\
"Frontend host = STRING : [32] localhost",\
"Frontend name = STRING : [32] SC Frontend",\
"Frontend file name = STRING : [256] /home/standard/daq/online/scfe.cxx",\
"Status = STRING : [256] Ok",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT32 : 0",\
"",\
NULL }

#define HV_SETTINGS_DEFINED

typedef struct {
  struct {
    struct {
      BOOL      enabled;
      struct {
        char      system_name[32];
        char      ip[32];
        INT32     linktype;
        INT32     first_slot;
        INT32     cratemap;
      } dd;
      struct {
        char      description[22];
        char      model[8];
        UINT16    channels;
      } slot_0;
      struct {
        char      description[22];
        char      model[6];
        UINT16    channels;
      } slot_10;
    } sy4527;
  } devices;
  char      editable[2][32];
  char      names[26][32];
  float     update_threshold_measured[26];
  float     update_threshold_current[26];
  float     zero_threshold[26];
  float     voltage_limit[26];
  float     current_limit[26];
  float     current_hot_spot[26];
  float     trip_time[26];
  float     ramp_up_speed[26];
  float     ramp_down_speed[26];
} HV_SETTINGS;

#define HV_SETTINGS_STR(_name) const char *_name[] = {\
"[Devices/sy4527]",\
"Enabled = BOOL : y",\
"",\
"[Devices/sy4527/DD]",\
"System Name = STRING : [32] daqhv02",\
"IP = STRING : [32] 192.168.0.102",\
"LinkType = INT32 : 0",\
"First Slot = INT32 : 0",\
"crateMap = INT32 : 0",\
"",\
"[Devices/sy4527/Slot 0]",\
"Description = STRING : [22]  14 Ch Float 1KV 1mA ",\
"Model = STRING : [8] A1515TG",\
"Channels = UINT16 : 14",\
"",\
"[Devices/sy4527/Slot 10]",\
"Description = STRING : [22]  12 Ch Neg. 3KV 3mA  ",\
"Model = STRING : [6] A1833",\
"Channels = UINT16 : 12",\
"",\
"[.]",\
"Editable = STRING[2] :",\
"[32] Demand",\
"[32] ChState",\
"Names = STRING[26] :",\
"[32] Offset",\
"[32] VGEM3",\
"[32] Transf2",\
"[32] VGEM2",\
"[32] Transf1",\
"[32] VGEM1",\
"[32] Drift",\
"[32] Ch00.007",\
"[32] Ch00.008",\
"[32] Ch00.009",\
"[32] Ch00.010",\
"[32] Ch00.011",\
"[32] Ch00.012",\
"[32] Ch00.013",\
"[32] PMT1",\
"[32] PMT2",\
"[32] PMT3",\
"[32] broken",\
"[32] PMT4",\
"[32] Ch10.005",\
"[32] Ch10.006",\
"[32] Ch10.007",\
"[32] Ch10.008",\
"[32] Ch10.009",\
"[32] Ch10.010",\
"[32] Ch10.011",\
"Update Threshold Measured = FLOAT[26] :",\
"[0] 0.5",\
"[1] 0.5",\
"[2] 0.5",\
"[3] 0.5",\
"[4] 0.5",\
"[5] 0.5",\
"[6] 0.5",\
"[7] 0.5",\
"[8] 0.5",\
"[9] 0.5",\
"[10] 0.5",\
"[11] 0.5",\
"[12] 0.5",\
"[13] 0.5",\
"[14] 0.5",\
"[15] 0.5",\
"[16] 0.5",\
"[17] 0.5",\
"[18] 0.5",\
"[19] 0.5",\
"[20] 0.5",\
"[21] 0.5",\
"[22] 0.5",\
"[23] 0.5",\
"[24] 0.5",\
"[25] 0.5",\
"Update Threshold Current = FLOAT[26] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"[8] 1",\
"[9] 1",\
"[10] 1",\
"[11] 1",\
"[12] 1",\
"[13] 1",\
"[14] 1",\
"[15] 1",\
"[16] 1",\
"[17] 1",\
"[18] 1",\
"[19] 1",\
"[20] 1",\
"[21] 1",\
"[22] 1",\
"[23] 1",\
"[24] 1",\
"[25] 1",\
"Zero Threshold = FLOAT[26] :",\
"[0] 20",\
"[1] 20",\
"[2] 20",\
"[3] 20",\
"[4] 20",\
"[5] 20",\
"[6] 20",\
"[7] 20",\
"[8] 20",\
"[9] 20",\
"[10] 20",\
"[11] 20",\
"[12] 20",\
"[13] 20",\
"[14] 20",\
"[15] 20",\
"[16] 20",\
"[17] 20",\
"[18] 20",\
"[19] 20",\
"[20] 20",\
"[21] 20",\
"[22] 20",\
"[23] 20",\
"[24] 20",\
"[25] 20",\
"Voltage Limit = FLOAT[26] :",\
"[0] 1000",\
"[1] 443",\
"[2] 505",\
"[3] 443",\
"[4] 505",\
"[5] 443",\
"[6] 1000",\
"[7] 100",\
"[8] 470",\
"[9] 655",\
"[10] 470",\
"[11] 505",\
"[12] 470",\
"[13] 10",\
"[14] 2550",\
"[15] 2500",\
"[16] 2550",\
"[17] 2550",\
"[18] 2660",\
"[19] 2800",\
"[20] 2800",\
"[21] 2800",\
"[22] 3000",\
"[23] 3000",\
"[24] 3000",\
"[25] 3000",\
"Current Limit = FLOAT[26] :",\
"[0] 10",\
"[1] 10",\
"[2] 10",\
"[3] 10",\
"[4] 10",\
"[5] 10",\
"[6] 10",\
"[7] 10",\
"[8] 10",\
"[9] 10",\
"[10] 10",\
"[11] 10",\
"[12] 10",\
"[13] 10",\
"[14] 2000",\
"[15] 2000",\
"[16] 2000",\
"[17] 2000",\
"[18] 2000",\
"[19] 2000",\
"[20] 2000",\
"[21] 2000",\
"[22] 2000",\
"[23] 2000",\
"[24] 2000",\
"[25] 0",\
"Current Hot Spot = FLOAT[26] :",\
"[0] 10",\
"[1] 10",\
"[2] 10",\
"[3] 10",\
"[4] 10",\
"[5] 10",\
"[6] 10",\
"[7] 10",\
"[8] 10",\
"[9] 10",\
"[10] 10",\
"[11] 10",\
"[12] 10",\
"[13] 10",\
"[14] 10",\
"[15] 10",\
"[16] 10",\
"[17] 10",\
"[18] 10",\
"[19] 10",\
"[20] 10",\
"[21] 10",\
"[22] 10",\
"[23] 10",\
"[24] 10",\
"[25] 10",\
"Trip Time = FLOAT[26] :",\
"[0] 10",\
"[1] 10",\
"[2] 10",\
"[3] 10",\
"[4] 10",\
"[5] 10",\
"[6] 100",\
"[7] 10",\
"[8] 10",\
"[9] 10",\
"[10] 10",\
"[11] 10",\
"[12] 10",\
"[13] 10",\
"[14] 999",\
"[15] 999",\
"[16] 999",\
"[17] 999",\
"[18] 999",\
"[19] 999",\
"[20] 999",\
"[21] 999",\
"[22] 999",\
"[23] 999",\
"[24] 999",\
"[25] 999",\
"Ramp Up Speed = FLOAT[26] :",\
"[0] 10",\
"[1] 10",\
"[2] 10",\
"[3] 10",\
"[4] 10",\
"[5] 10",\
"[6] 10",\
"[7] 5",\
"[8] 5",\
"[9] 5",\
"[10] 5",\
"[11] 5",\
"[12] 5",\
"[13] 5",\
"[14] 100",\
"[15] 100",\
"[16] 100",\
"[17] 100",\
"[18] 100",\
"[19] 200",\
"[20] 200",\
"[21] 200",\
"[22] 200",\
"[23] 100",\
"[24] 100",\
"[25] 100",\
"Ramp Down Speed = FLOAT[26] :",\
"[0] 20",\
"[1] 20",\
"[2] 20",\
"[3] 20",\
"[4] 20",\
"[5] 20",\
"[6] 20",\
"[7] 20",\
"[8] 20",\
"[9] 20",\
"[10] 20",\
"[11] 20",\
"[12] 20",\
"[13] 20",\
"[14] 200",\
"[15] 200",\
"[16] 200",\
"[17] 200",\
"[18] 200",\
"[19] 200",\
"[20] 100",\
"[21] 100",\
"[22] 200",\
"[23] 100",\
"[24] 100",\
"[25] 200",\
"",\
NULL }

#endif

#ifndef EXCL_ENVIRONMENT

#define ENVIRONMENT_COMMON_DEFINED

typedef struct {
  UINT16    event_id;
  UINT16    trigger_mask;
  char      buffer[32];
  INT32     type;
  INT32     source;
  char      format[8];
  BOOL      enabled;
  INT32     read_on;
  INT32     period;
  double    event_limit;
  UINT32    num_subevents;
  INT32     log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT32     write_cache_size;
} ENVIRONMENT_COMMON;

#define ENVIRONMENT_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = UINT16 : 5",\
"Trigger mask = UINT16 : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT32 : 16",\
"Source = INT32 : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT32 : 255",\
"Period = INT32 : 60000",\
"Event limit = DOUBLE : 0",\
"Num subevents = UINT32 : 0",\
"Log history = INT32 : 1",\
"Frontend host = STRING : [32] localhost",\
"Frontend name = STRING : [32] SC Frontend",\
"Frontend file name = STRING : [256] /home/standard/daq/online/scfe.cxx",\
"Status = STRING : [256] Partially disabled",\
"Status color = STRING : [32] yellowGreenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT32 : 0",\
"",\
NULL }

#define ENVIRONMENT_SETTINGS_DEFINED

typedef struct {
  struct {
    struct {
      char      mscb_device[32];
      char      mscb_pwd[32];
      INT32     mscb_address[27];
      UINT8     mscb_index[27];
      BOOL      enabled;
      INT32     mscb_debug;
      INT32     mscb_pause;
      INT32     mscb_retries;
      char      device[256];
    } input;
    struct {
      BOOL      enabled;
    } output;
  } devices;
  float     update_threshold[27];
  float     input_offset[27];
  float     input_factor[27];
  char      names_input[27][32];
} ENVIRONMENT_SETTINGS;

#define ENVIRONMENT_SETTINGS_STR(_name) const char *_name[] = {\
"[Devices/Input]",\
"MSCB Device = STRING : [32] 192.168.0.103",\
"MSCB Pwd = STRING : [32] meg",\
"MSCB Address = INT32[27] :",\
"[0] 65535",\
"[1] 65535",\
"[2] 65535",\
"[3] 65535",\
"[4] 65535",\
"[5] 65535",\
"[6] 65535",\
"[7] 65535",\
"[8] 65535",\
"[9] 65535",\
"[10] 65535",\
"[11] 65535",\
"[12] 65535",\
"[13] 65535",\
"[14] 65535",\
"[15] 65535",\
"[16] 65535",\
"[17] 65535",\
"[18] 65535",\
"[19] 65535",\
"[20] 65535",\
"[21] 65535",\
"[22] 65535",\
"[23] 65535",\
"[24] 65535",\
"[25] 65535",\
"[26] 65535",\
"MSCB Index = UINT8[27] :",\
"[0] 0",\
"[1] 1",\
"[2] 2",\
"[3] 3",\
"[4] 4",\
"[5] 5",\
"[6] 6",\
"[7] 7",\
"[8] 8",\
"[9] 9",\
"[10] 10",\
"[11] 11",\
"[12] 12",\
"[13] 13",\
"[14] 14",\
"[15] 15",\
"[16] 16",\
"[17] 17",\
"[18] 18",\
"[19] 19",\
"[20] 20",\
"[21] 21",\
"[22] 22",\
"[23] 23",\
"[24] 24",\
"[25] 25",\
"[26] 26",\
"Enabled = BOOL : y",\
"MSCB Debug = INT32 : 0",\
"MSCB Pause = INT32 : 0",\
"MSCB Retries = INT32 : 10",\
"Device = STRING : [256] 192.168.0.103",\
"",\
"[Devices/Output]",\
"Enabled = BOOL : y",\
"",\
"[.]",\
"Update Threshold = FLOAT[27] :",\
"[0] 0.1",\
"[1] 0.1",\
"[2] 0.1",\
"[3] 0.1",\
"[4] 0.1",\
"[5] 0.1",\
"[6] 0.1",\
"[7] 0.1",\
"[8] 0.1",\
"[9] 0.1",\
"[10] 0.1",\
"[11] 0.1",\
"[12] 0.1",\
"[13] 0.1",\
"[14] 0.1",\
"[15] 0.1",\
"[16] 0.1",\
"[17] 0.1",\
"[18] 0.1",\
"[19] 0.1",\
"[20] 0.1",\
"[21] 0.1",\
"[22] 0.1",\
"[23] 0.1",\
"[24] 0.1",\
"[25] 0.1",\
"[26] 0.1",\
"Input Offset = FLOAT[27] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"[16] 0",\
"[17] 0",\
"[18] 0",\
"[19] 0",\
"[20] 0",\
"[21] 0",\
"[22] 0",\
"[23] 0",\
"[24] 0",\
"[25] 0",\
"[26] 0",\
"Input Factor = FLOAT[27] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"[8] 1",\
"[9] 1",\
"[10] 1",\
"[11] 1",\
"[12] 1",\
"[13] 1",\
"[14] 1",\
"[15] 1",\
"[16] 1",\
"[17] 1",\
"[18] 1",\
"[19] 1",\
"[20] 1",\
"[21] 1",\
"[22] 1",\
"[23] 1",\
"[24] 1",\
"[25] 1",\
"[26] 1",\
"Names Input = STRING[27] :",\
"[32] P0IIn0",\
"[32] P0IIn1",\
"[32] P0IIn2",\
"[32] P0IIn3",\
"[32] P0IIn4",\
"[32] P0IIn5",\
"[32] P0IIn6",\
"[32] P0IIn7",\
"[32] P0Calib",\
"[32] P1UIn0",\
"[32] P1UIn1",\
"[32] P1UIn2",\
"[32] P1UIn3",\
"[32] P1UIn4",\
"[32] P1UIn5",\
"[32] P1UIn6",\
"[32] P1UIn7",\
"[32] P1Calib",\
"[32] P3IIn0",\
"[32] P3IIn1",\
"[32] P3IIn2",\
"[32] P3IIn3",\
"[32] P3IIn4",\
"[32] P3IIn5",\
"[32] P3IIn6",\
"[32] P3IIn7",\
"[32] P3Calib",\
"",\
NULL }

#endif

