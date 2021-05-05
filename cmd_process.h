// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include"service.h"

void run_launch( ucmd *cmd );
void run_kill( ucmd *cmd );
void startObserving( channelT *chan, int32_t pid );
uint64_t launchProcess(
  void *device,
  char *bundleIdentifier,
  tDICT *environment,
  tARR *arguments,
  tDICT *options
);

uint64_t launchProcess_withchan(
  void *device,
  char *bundleIdentifier,
  tDICT *environment,
  tARR *arguments,
  tDICT *options,
  channelT *chan
);

channelT *channel__new_processcontrol( void *device );