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
sub serverdone {
	return 1;
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
			if($ircinput =~ /^pass\s*/i || $ircinput =~ /^user\s*/i) {
				# ignore
			} elsif($ircinput =~ /^nick\s+(.+)$/i) {
				my $nick = $1;
				print $irc ":server NOTICE Auth :An AUTH Message\r\n";
				print $irc ":server 001 $nick :Welcome to this test server\r\n";
				print $irc ":server 375 $nick :server message of the day\r\n";
				print $irc ":server 372 $nick :- MOTD\r\n";
				print $irc ":server 376 $nick :End of message of the day.\r\n";
				print $irc ":$nick MODE $nick +x\r\n";
				print $irc ":server NOTICE $nick :A notice mE5sage\r\n";
				my $channel = "##Ch4nN3l";
				print $irc ":$nick JOIN :$channel\r\n";
				print $irc ":server 332 $nick $channel :A T0p1C:!\r\n";
				print $irc ":server 333 $nick $channel $nick 1336038237\r\n";
				print $irc ":server 353 $nick = $channel :\@Op3rAT0R +V01CE ~OwN3R Norm4L $nick\r\n";
				print $irc ":server 366 $nick $channel :End of names list\r\n";
				print $irc ":t3ST PRIVMSG $nick :Hell0 thEre!\r\n";
				print $irc ":f0O NOTICE $channel :Not1c3\r\n";
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
