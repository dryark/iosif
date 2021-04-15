void run_launch( ucmd *cmd );
void run_kill( ucmd *cmd );
uint64_t launchProcess(
  void *device,
  char *bundleIdentifier,
  tDICT *environment,
  tARR *arguments,
  tDICT *options
);