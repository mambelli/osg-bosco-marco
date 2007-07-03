#! /usr/bin/env perl
use CondorTest;

$cmd      = 'job_filexfer_trans-excut-true_van.cmd';
$testname = 'Jobs complaining cause No FT on when tansfer_executables = true - vanilla U';

# truly const variables in perl
sub IDLE{1};
sub HELD{5};
sub RUNNING{2};

my $alreadydone=0;

$execute = sub {
	my %args = @_;
	my $cluster = $args{"cluster"};

	print "Running $cluster\n";

};

$abort = sub {
	die "Job is gone now, cool.\n";
};

$hold = sub
{
	my %args = @_;
	my $cluster = $args{"cluster"};

	print "Good. Cluster $cluster is supposed to be held.....\n";
};

$release = sub
{
	my %args = @_;
	my $cluster = $args{"cluster"};

};

$success = sub
{
	die "Base file transfer job!\n";
};

$wanterror = sub
{
	print "Base file transfer job, submit supposed to fail...error 1 ..!\n";
	my %args = @_;
	my $errmsg = $args{"ErrorMessage"};

    if($errmsg =~ /^.*died abnormally.*$/) {
        print "BAD. Submit died was to fail but with error 1\n";
        print "$testname: Failure\n";
        exit(1);
    } elsif($errmsg =~ /^.*\(\s*returned\s*(\d+)\s*\).*$/) {
        if($1 == 1) {
            print "Good. Job was not to submit with File Transfer off and input files requested\n";
            print "$testname: SUCCESS\n";
            exit(0);
        } else {
            print "BAD. Submit was to fail but with error 1 not <<$1>>\n";
            print "$testname: Failure\n";
            exit(1);
        }
    } else {
            print "BAD. Submit failure mode unexpected....\n";
            print "$testname: Failure\n";
            exit(1);
    }
};

$timed = sub
{
	die "Job should have ended by now. file transfer broken!\n";
};

# make some needed files. All 0 sized and xxxxxx.txt for
# easy cleanup

system("touch submit_filetrans_input.txt");
system("touch submit_filetrans_inputa.txt");
system("touch submit_filetrans_inputb.txt");
#system("touch submit_filetrans_inputc.txt");

CondorTest::RegisterExecute($testname, $execute);
CondorTest::RegisterWantError($testname, $wanterror);
CondorTest::RegisterAbort($testname, $abort);
CondorTest::RegisterHold($testname, $hold);
CondorTest::RegisterRelease($testname, $release);
CondorTest::RegisterExitedSuccess($testname, $success);
CondorTest::RegisterTimed($testname, $timed, 3600);

if( CondorTest::RunTest($testname, $cmd, 0) ) {
	print "$testname: SUCCESS\n";
	exit(0);
} else {
	die "$testname: CondorTest::RunTest() failed\n";
}
