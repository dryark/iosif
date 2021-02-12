// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

void runInfo( void *device );
void run_info( ucmd *cmd ) { g_cmd = cmd; waitForConnect( runInfo ); }

void runInfo( void *device ) {
  char useJson = ucmd__get( g_cmd, "-json" ) ? 1 : 0;
  
  devUp( device );
  
  if( useJson ) printf("{\n");
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
      if( val ) {
        if( useJson ) {
          printf("%s:",name);
          cf2json( val );
          printf(",\n");
        }
        else {
          if( g_cmd->argc>1) printf("%s:",name);
          cfdump( 0, val );
        }
      }
      free( name );
  }
  if( useJson ) printf("}\n");
  
  devDown( device );
  
  exit(0);
}