#!/usr/bin/perl -w

# Copyright (c) 2021 David Helkowski
# Anti-Corruption License

use strict;
use XML::Bare qw/forcearray/;

print <<END;
<head>
  <style>
    body {
      background: #aaa;
    }
    .msg {
      background: white;
      border: solid 1px black;
      margin-bottom: 10px;
      padding: 5px;
      display: inline-block;
    }
    .data {
      background: lightblue;
      white-space: pre;
      display: inline-block;
      max-height: 400px;
      max-width: 800px;
      overflow-y: scroll;
      padding: 2px;
      margin: 2px;
    }
    .arg {
      border: solid 1px black;
      white-space: pre;
      padding: 2px;
      margin: 2px;
      display: inline-block;
      max-height: 400px;
      max-width: 800px;
      
      overflow-y: scroll;
    }
  </style>
</head>
END

my $xmlfile = $ARGV[0];
#print "File:$xmlfile\n";

my ( $ob, $root ) = XML::Bare->new( file => $xmlfile );

my $msgs = forcearray $root->{msg};

sub noteAcked {
  my ( $msgs, $startI, $wantId, $dirWant ) = @_;
  for( my $i=$startI;$i>=0;$i-- ) {
    my $msg = $msgs->[ $i ];
    my $id = $msg->{id}{value};
    if( $id eq $wantId ) {
      $msg->{acked} = 1;
      return 1;
    }
  }
  return 0;
}

my $i = 0;
for my $msg ( @$msgs ) {
  my $id = $msg->{id}{value};
  
  my $id1;
  my $id2;
  my $id3;
  if( $id =~ m/(.+)\.(.+)\.(.+)/ ) {
    $id1 = $1;
    $id2 = $2;
    $id3 = $3;
  } else {
    next;
  }
  my $rid3 = $id3-1;
  my $findId = "$id1.$id2.$rid3";
  
  my $type = $msg->{type}{value};
  if( $type eq 'ack' ) {
    #print "Looking for $findId\n";
    my $dirWant = "in";
    my $acked = noteAcked( $msgs, $i-1, $findId, $dirWant );
    $msg->{done} = 1 if( $acked );
  }
  if( $type eq 'rack' ) {
    #print "Looking for $findId\n";
    my $dirWant = "out";
    my $acked = noteAcked( $msgs, $i-1, $findId, $dirWant );
    $msg->{done} = 1 if( $acked );
  }
  $i++;
}

for my $msg ( @$msgs ) {
  next if( $msg->{done} );
  my $type = $msg->{type}{value};
  my $dir = $msg->{dir} ? $msg->{dir}{value} : 0;
  if( $type eq 'ack' ) { $dir = "out"; }
  if( $type eq 'rack' ) { $dir = "in"; }
  my $id = $msg->{id}{value};
  my $itype = $msg->{itype} ? $msg->{itype}{value} : 0;
  
  my $msgt = $msg->{msg} ? $msg->{msg}{value} : "";
  if( $msgt =~ m/^"(.+)"$/ ) {
    $msgt = $1;
  }
  #if( $msgt =~ m/(.+)\n$/ ) {
  #  $msgt = $1;
  #}
  
  print "<div class='msg'>$type $dir $id\n";
  if( $msgt ) { print "<br><div class='data'>$msgt</div><br>\n"; }
  if( $msg->{acked} ) { print "ACKED<br>"; }
  
  if( $msg->{arg} ) {
    my $args = forcearray $msg->{arg};
    #print "Args:\n";
    for my $argn ( @$args ) {
      my $arg = $argn->{value};
      if( $arg =~ m/^"(.+)\n?"$/ ) {
        $arg = $1;
      }
      if( $arg =~ m/(.+)\n$/s ) {
        $arg = $1;
      }
      print "<div class='arg'>$arg</div><br>\n";
    }
  }
  
  print "</div><br>\n";
}