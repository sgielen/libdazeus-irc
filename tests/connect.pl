#!/usr/bin/perl
use strict;
use warnings;
use Socket;
use IO::Socket::INET;
use Fcntl qw(F_GETFL F_SETFL O_NONBLOCK);

$|++;

# Run an IRC server that allows a connection; this test succeeds
# if a connection arrives on the IRC server with the correct
# parameters, and the connect process itself exits with 0.

sub debugEnabled {
	return $ENV{TEST_DEBUG};
}

sub debug {
	print $_[0] . "\n" if(debugEnabled());
}

if(@ARGV < 1) {
	die "Usage: $0 testexec [testparam1 [testparam2 [...]]]\n";
}

my $exec = shift @ARGV;
if(! -r $exec) {
	warn "File not found: $exec\n";
	die "Usage: $0 textexec [testparam1 [testparam2 [...]]]\n";
}

sub set_nonblock {
	my $flags = fcntl($_[0], F_GETFL, 0) or die $!;
	$flags = fcntl($_[0], F_SETFL, $flags | O_NONBLOCK) or die $!;
}

my ($chld, $prnt);
socketpair($chld, $prnt, AF_UNIX, SOCK_STREAM, PF_UNSPEC) or die $!;

my $stdout = select $chld;
$|++;
select $prnt;
$|++;
select $stdout;

my $host = "localhost";
my $port = int(rand(10000)) + 30000;

debug("Starting test. Host: $host, port: $port");

my $pid = fork();
if($pid == 0) {
	close $chld;
	# we are the child; sleep for a few seconds, then run the connect
	# process until it returns
	debug("[C] Waiting for parent...");
	unless(debugEnabled()) {
		close(STDOUT);
		close(STDERR);
	}
	while(<$prnt>) {
		last if($_ eq "start\n");
	}
	local $SIG{ALRM} = sub { print $prnt "timeout\n"; exit };
	alarm 5;
	debug("[C] Parent is listening. Starting child process.");
	my $res = system($exec, $host, $port, @ARGV);
	alarm 0;
	print $prnt "res $res\n";
	debug("[C] Child process done; returned $res.");
	exit;
}

close $prnt;

debug("[P] Starting listening socket...");

my $ircd = IO::Socket::INET->new(
	LocalAddr => $host,
	LocalPort => $port,
	Listen => 1,
	Proto => "tcp",
	Type => SOCK_STREAM,
	Blocking => 1,
);

if(!$ircd) {
	warn "# Couldn't create listen socket: $!\n";
	waitpid($pid, 0);
	exit 1;
}

debug("[P] Listening. Waiting for child.");
print $chld "start\n";

my $childdone = 0;
my $serverdone = 0;
my ($nick, $user, $pwd) = (0, 0, 0);
sub serverdone {
	return $nick == 1 && $user == 1 && $pwd == 1;
}
sub killchild {
	kill(9, $pid); waitpid($pid, 0);
}

eval {
	local $SIG{ALRM} = sub { warn "# Timeout\n"; killchild(); exit 2; };
	alarm 10;
	my $irc = $ircd->accept();
	debug("[P] Accepted socket.");
	set_nonblock($irc);
	set_nonblock($chld);
	while(1) {
		my $childinput = <$chld>;
		if($childinput) {
			if($childinput =~ /^timeout/) {
				warn "# Child timeout\n";
				killchild();
				exit 3;
			} elsif($childinput =~ /^res (.*)$/) {
				if($1 != 0) {
					warn "# Child failure\n";
					killchild();
					exit $1;
				}
				$childdone = 1;
			} else {
				warn "# Child input not understood: $childinput\n";
			}
		}
		my $ircinput = <$irc> if $irc;
		if($ircinput) {
			$ircinput =~ s/[\n\r]+//g;
			if($ircinput =~ /^pass\s?(.+)?$/i) {
				if(!$1 || $1 ne "p4sSW0rd") {
					warn "# Password incorrect\n";
					killchild();
					exit 4;
				}
				$pwd = 1;
			} elsif($ircinput =~ /^nick (.+)$/i) {
				if($1 ne "n1CKn4M3") {
					warn "# Nickname incorrect\n";
					killchild();
					exit 4;
				}
				$nick = 1;
			} elsif($ircinput =~ /^user (.+?) (.+?) (.+?) :(.+?)$/i) {
				if($1 ne "us3RN4m3") {
					warn "# Username incorrect\n";
					killchild();
					exit 4;
				} elsif($4 ne "fuLLn4m3") {
					warn "# Fullname incorrect\n";
					killchild();
					exit 4;
				}
				$user = 1;
			} else {
				warn "# IRC input not understood: $ircinput\n";
			}
		}

		if(serverdone() && !$childdone && $irc) {
			# Disconnect so the child can exit
			print $irc ":server 001 n1CKn4M3 :Welcome to this test server\n";
			print $irc ":server 375 n1CKn4M3 :server message of the day\n";
			print $irc ":server 372 n1CKn4M3 :- MOTD\n";
			print $irc ":server 376 n1CKn4M3 :End of message of the day.\n";
			print $irc "ERROR :Disconnecting you.\n";
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

killchild();
exit 0;
