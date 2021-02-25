// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"uclop.h"

#include"cmd_ioreg.h"
#include"cmd_install.h"
#include"cmd_tunnel.h"
#include"cmd_bytetest.h"
#include"cmd_syscfg.h"
#include"cmd_info.h"
#include"cmd_gg.h"
#include"cmd_mg.h"
#include"cmd_img.h"
#include"cmd_list.h"
#include"cmd_syslog.h"
#include"service_deviceinfo.h"
#include"service_notifications.h"
#include"service_sysmon.h"

int main (int argc, char **argv) {
  uopt *idopt = UOPT("-id","UDID of device");
  
  uopt *opts_list[] = {
    UOPT_FLAG("-json","Output JSON format"), 0
  };
  uopt *udid_option[]        = { idopt, 0 };
  uopt *udid_w_json_option[] = { UOPT_FLAG("-json","Output JSON format"), idopt, 0 };
  uopt *opts_install[]       = { UOPT_REQUIRED("-path","Path to .app"), idopt, 0 };
  uopt *opts_ps[] = {
    UOPT_FLAG("-subsec","Subsecond start precision"),
    UOPT_FLAG("-short","Show name only, not full path"),
    UOPT_FLAG("-apps","Show apps only"),
    idopt, 0
  };
  
  uclop *opts = uclop__new( 0, 0 );
  uclop__addcmd( opts, "log",          "Syslog",                      &run_syslog,       udid_option );
  uclop__addcmd( opts, "list",         "List Devices",                &run_list,         opts_list );
  uclop__addcmd( opts, "detectloop",   "Detect Loop",                 &run_detect,       0 );
  uclop__addcmd( opts, "img",          "Get screenshot",              &run_img,          0 );
  uclop__addcmd( opts, "gas",          "Gas Guage",                   &run_gg,           0 );
  uclop__addcmd( opts, "syscfg",       "Get SysCfg",                  &run_syscfg,       0 );
  uclop__addcmd( opts, "ps",           "Process list",                &run_ps,           opts_ps );
  uclop__addcmd( opts, "btest",        "Bytearr Test",                &run_bytetest,     0 );
  uclop__addcmd( opts, "sysmon",       "Sysmontap",                   &run_sysmon,       0 );
  uclop__addcmd( opts, "smproclist",   "Sysmon Process Attributes",   &run_smProcList,   0 );
  uclop__addcmd( opts, "smsyslist",    "Sysmon System Attributes",    &run_smSysList,    0 );
  uclop__addcmd( opts, "smcoallist",   "Sysmon Coalition Attributes", &run_smCoalList,   0 );
  uclop__addcmd( opts, "machtimeinfo", "Mach Time Info",              &run_machTimeInfo, 0 );
  uclop__addcmd( opts, "notices",      "Mobile Notifications",        &run_notices,      0 );
  uclop__addcmd( opts, "battery",      "Battery Info",                &run_battery,      0 );
  uclop__addcmd( opts, "install",      "Install Application",         &run_install,      opts_install );
  
  ucmd *mg = uclop__addcmd( opts, "mg", "Mobile Gestalt", &run_mg, udid_w_json_option );
  mg->extrahelp = "[key] [[key]...]";
  
  ucmd *info = uclop__addcmd( opts, "info", "Info", &run_info, udid_w_json_option );
  info->extrahelp = "[[domain:]key] [[key]...]";
    
  ucmd *ls = uclop__addcmd( opts, "ls", "ls", &run_ls, 0 );
  ls->extrahelp = "[path]";
  
  ucmd *ioreg = uclop__addcmd( opts, "ioreg", "IO Registry", &run_ioreg, 0 );
  ioreg->extrahelp = "[key]";
    
  ucmd *tun = uclop__addcmd( opts, "tunnel", "Tunnel", &run_tunnel, udid_option );
  tun->extrahelp = "[from]:[to] [[from]:[to]...]";
  
  uclop__run( opts, argc, argv );
}