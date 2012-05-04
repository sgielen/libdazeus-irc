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
my ($nick, $user, $pwd) = (0, 0, 0);
sub serverdone {
	return $nick == 1 && $user == 1 && $pwd == 1;
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
			if($ircinput =~ /^pass\s?(.+)?$/i) {
				if(!$1 || $1 ne "p4sSW0rd") {
					warn "# Password incorrect\n";
					stopTest($pid);
					exit 4;
				}
				$pwd = 1;
			} elsif($ircinput =~ /^nick (.+)$/i) {
				if($1 ne "n1CKn4M3") {
					warn "# Nickname incorrect\n";
					stopTest($pid);
					exit 4;
				}
				$nick = 1;
			} elsif($ircinput =~ /^user (.+?) (.+?) (.+?) :(.+?)$/i) {
				if($1 ne "us3RN4m3") {
					warn "# Username incorrect\n";
					stopTest($pid);
					exit 4;
				} elsif($4 ne "fuLLn4m3") {
					warn "# Fullname incorrect\n";
					stopTest($pid);
					exit 4;
				}
				$user = 1;
			} else {
				warn "# IRC input not understood: $ircinput\n";
			}
		}

		if(serverdone() && !$childdone && $irc) {
			# Disconnect so the child can exit
			close $irc;
			undef $irc;
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
