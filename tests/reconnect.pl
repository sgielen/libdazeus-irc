#!/usr/bin/perl
use strict;
use warnings;
use lib "../tests";
use DaZeusTest;

# Run an IRC server that allows a connection; this test succeeds
# if a connection arrives on the IRC server with the correct
# parameters, and the connect process itself exits with 0.

my ($chld, $ircd, $pid) = startTest(@ARGV);

my $childdone = 0;
my ($connectone, $connecttwo) = (0, 0);
sub serverdone {
	return $connectone == 1 && $connecttwo == 1
}

eval {
	local $SIG{ALRM} = sub { warn "# Timeout\n"; stopTest($pid); exit 2; };
	alarm 10;
	my $irc = $ircd->accept();
	debug("[P] Accepted socket.");
	set_nonblock($irc);
	set_nonblock($chld);
	while(1) {
		handle_child($chld, $pid, \$childdone);
		my $ircinput = <$irc> if $irc;
		if($ircinput) {
			$ircinput =~ s/[\n\r]+//g;
			if($ircinput =~ /^pass/i || $ircinput =~ /^user/i) {
				# that's ok
			} elsif($ircinput =~ /^nick (.+)$/i) {
				if($1 eq "connectone" && !$connectone) {
					$connectone = 1;
					debug("First connect registered. Allowing on network, then disconnecting");
					print $irc ":server 001 connectone :Welcome to this test server\n";
					print $irc ":server 375 connectone :server message of the day\n";
					print $irc ":server 372 connectone :- MOTD\n";
					print $irc ":server 376 connectone :End of message of the day.\n";
					print $irc "ERROR :disconnecting you\n";
					close $irc;
					$irc = $ircd->accept();
					debug("New connection accepted.");
				} elsif($1 eq "connectone" && $connectone) {
					warn "# First connect happened twice. Failing test.\n";
					stopTest($pid);
					exit 4;
				} elsif($1 eq "connecttwo" && !$connectone) {
					warn "# Missed first connect. Failing test.\n";
					stopTest($pid);
					exit 5;
				} elsif($1 eq "connecttwo" && !$connecttwo) {
					$connecttwo = 1;
					debug("Second connect registered; reconnection worked. Succeeding test.");
					print $irc ":server 001 connectone :Welcome to this test server\n";
					print $irc ":server 375 connectone :server message of the day\n";
					print $irc ":server 372 connectone :- MOTD\n";
					print $irc ":server 376 connectone :End of message of the day.\n";
					print $irc "ERROR :disconnecting you\n";
					close $irc;
					undef $irc;
				} elsif($1 eq "connecttwo" && $connecttwo) {
					warn "# Second connect happened twice. Failing test.\n";
					stopTest($pid);
					exit 6;
				}
			} else {
				warn "# IRC input not understood: $ircinput\n";
			}
		}

		if($childdone && serverdone()) {
			debug("Child and server are both done\n");
			# Success
			last;
		}
	}
	alarm 0;
};

if($@) {
	die $@;
}

stopTest($pid);
exit 0;
