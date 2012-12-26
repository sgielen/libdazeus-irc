package DaZeusTest;
use strict;
use warnings;
use Socket;
use IO::Socket::INET;
use Fcntl qw(F_GETFL F_SETFL O_NONBLOCK);
require Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(debugEnabled debug startTest stopTest set_nonblock handle_child);

sub debugEnabled {
	return $ENV{TEST_DEBUG};
}

sub debug {
	print $_[0] . "\n" if(debugEnabled());
}

sub set_nonblock {
	my $flags = fcntl($_[0], F_GETFL, 0) or die $!;
	$flags = fcntl($_[0], F_SETFL, $flags | O_NONBLOCK) or die $!;
}

sub stopTest {
	kill(9, $_[0]); waitpid($_[0], 0);
}

sub startTest {
	if(@ARGV < 1) {
		die "Usage: $0 testexec [testparam1 [testparam2 [...]]]\n";
	}

	my $exec = shift @ARGV;
	if(! -r $exec) {
		warn "File not found: $exec\n";
		die "Usage: $0 textexec [testparam1 [testparam2 [...]]]\n";
	}

	my ($chld, $prnt);
	socketpair($chld, $prnt, AF_UNIX, SOCK_STREAM, PF_UNSPEC) or die $!;

	$|++;
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
		stopTest($pid);
		return;
	}

	debug("[P] Listening. Waiting for child.");
	print $chld "start\n";

	return ($chld, $ircd, $pid);
}

sub handle_child {
	my ($chld, $pid, $childdone) = @_;
	my $childinput = <$chld>;
	if($childinput) {
		if($childinput =~ /^timeout/) {
			warn "# Child timeout\n";
			stopTest($pid);
			exit 3;
		} elsif($childinput =~ /^res (.*)$/) {
			my $exit = $1;
			if($exit != 0) {
				warn "# Child failure\n";
				stopTest($pid);
				exit 1;
			}
			$$childdone = 1;
		} else {
			warn "# Child input not understood: $childinput\n";
		}
	}
}

1;
