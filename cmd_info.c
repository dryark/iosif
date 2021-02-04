// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

void runInfo( void *device );
void run_info( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runInfo ); }

void runInfo( void *device ) {
  devUp( device );
  
  for( int i=0;i<g_cmd->argc;i++ ) {
      char *name = strdup(g_cmd->argv[i]);
      char *dom = NULL;
      char *name2 = name;
      for( int j=0;1;j++ ) {
          char let = name[j];
          if( !let ) break;
          if( let == ':' ) {
              name[j] = 0x00;
              dom = name;
              name2 = &name[j+1];
          }
      }
      //printf("Dom=%s Name=%s\n", dom, name2 );
      CFTypeRef val = AMDeviceCopyValue( device, dom ? str_c2cf( dom ) : NULL, (name2[0]==0)?NULL:str_c2cf(name2) );
      free( name );
      cfdump( 0, val );
  }
  
  devDown( device );
  
  exit(0);
}