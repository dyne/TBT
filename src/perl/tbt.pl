#!/usr/bin/perl
# GPL and everything
# 2008 Robin Gareus <robin@gareus.org>
use Term::ReadKey;
use Term::Cap;
use Time::HiRes qw(gettimeofday);
use POSIX;
use Data::Dumper;
ReadMode('cbreak');

print "Time based text: User Ctrl-D on a new line to quit.\n";

@tbt=();
$prev=gettimeofday();
$mod=0;
while(1) {
  $c= ReadKey(0);
  $c=~y/\177\423\012/\010\010\15/;
  $o=ord($c);
  if ($mod==2) {
    if ($o==65) { $o=257; } # UP
    if ($o==66) { $o=258; } # DOWN
    if ($o==67) { $o=259; } # LEFT
    if ($o==68) { $o=260; } # RIGHT
  }

  if    ($mod==0 && $o==27) { $mod=1; } # UP
  elsif ($mod==1 && $o==91) { $mod=2; }
  else {$mod=0;}

  #print '-'.$o.'-'."\n";
  #next;
  $now=gettimeofday();
  last unless defined $c;
  last if ord($c)==4;
  $t=ceil(1000.0*($now-$prev));
  $prev=$now;
  if ($mod==0) { push @tbt, [$o, $t]; }
  if    ($o==8)  { print $c.' '.$c;} 
  elsif ($o==13) { print "\n";}
  else           { print ($c); }
}
ReadMode('normal');
print "\n---\n";

$first=0;
print "{{tbt>[";
foreach (@tbt) {
  if ($first) { print ',';} else { $first=1; }
  print '['.$_->[0].','.$_->[1].']';
}
print "]}}\n";
