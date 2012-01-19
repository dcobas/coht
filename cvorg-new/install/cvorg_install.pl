#!/usr/bin/perl
# Install cvorg modules from a transfer.ref file.
# Based on install script of sis33 modules created by Emilio G. Cota
#
# first the transfer.ref file is parsed and then, after some sanity checks,
# insmod is called with the parameters extracted from transfer.ref.
#
# call with 'script.pl --type 00' to install cvorg modules only.
# by default it tries to install all the modules it finds.

use strict;
use warnings;

use Getopt::Long;
use Pod::Usage;

my $device = 'CVORG';
my $driver = 'cvorg';
my $help;
my $man;
my $path = '/etc/transfer.ref';
my @AoH;
my @keys = ('ln', 'mln', 'bus', 'mtno', 'module-type', 'lu', 'W1', 'AM1',
	    'DPsz1', 'basaddr1', 'range1', 'W2', 'AM2', 'DPsz2', 'basaddr2',
	    'range2', 'testoff', 'sz', 'sl', 'ss', 'L', 'Vect');

my %base_addrs;

GetOptions(
	   "help|?|h"=> \$help,
	   "device=s"=> \$device,
	   "man"=> \$man,
	   "transfer_ref=s"=> \$path,
	   ) or pod2usage(2);

pod2usage(-exitstatus => 0, -verbose => 2) if $man;

open(INPUT, "<$path") or die ("$path not found");
# put all the described modules in an array of hashes (AoH)
LINE: while (<INPUT>) {
    next LINE if $_ !~ m|^\#\+\#|;

    chomp;
    my @values = split(/\s+/, $_);
    # remove the first '#+#'
    shift @values;

    my $href;
    foreach (@keys) {
	$href->{$_} = shift @values;
    }

    push @AoH, $href;
    $base_addrs{$href->{'ln'}} = $href->{'basaddr1'};
}
close(INPUT);

# If we found at least a module, then cvorg.ko should be installed
# But if the cvorg module is already there, we won't install it again
if (@AoH) {
    if (!module_is_loaded($driver)) {
	cvorg_install(\@AoH);
    }
} else {
    die "No $driver modules found in $path.\n";
}

sub module_is_loaded {
    my $module = shift;

    open(LSMOD, '/proc/modules') or die('Cannot open /proc/modules.\n');
    while (<LSMOD>) {
	chomp;
	my ($modname, $rest) = split(/\s+/, $_);

	if ($modname =~ m/^$module$/) {
	    close(LSMOD);
	    return 1;
	}
    }
    close(LSMOD);
    return 0;
}

sub cvorg_install {
    my $AoHref = shift;
    my $index_parm;
    my $base_address;
    my $irq;

    my $hrefProc;

  ENTRY: foreach my $href (@{$AoHref}) {
      next ENTRY if $href->{'module-type'} !~ m/$device/;

      if (defined $index_parm) {
	  $index_parm = $index_parm.','.$href->{'lu'};
	  $base_address = $base_address.',0x'.$href->{'basaddr1'};
          $irq = $irq.','.$href->{'Vect'};
      } else {
	  $index_parm = $href->{'lu'};
	  $base_address = '0x'.$href->{'basaddr1'};
          $irq = $href->{'Vect'};
      }

  }
    if (defined $index_parm) {
	my $driver_name = $driver.".ko";
	my $insmod = "insmod $driver_name lun=$index_parm base_address=$base_address irq=$irq";
	print $insmod, "\n";
	system($insmod) == 0 or die("$insmod failed");
    }
}

__END__

=head1 NAME

cvorg_install.pl - Install the CVORG kernel module using transfer.ref file.

=head1 SYNOPSIS

cvorg_install.pl [OPTIONS] driver

cvorg_install.pl - Install the CVORG kernel module using B<transfer.ref> file.

=head1 OPTIONS

=over 8

=item B<--device>

Name of the device(s) controlled by B<driver>. The default value is that of B<driver>.

=item B<--help -h -?>

Print a brief help message and exits.

=item B<--man>

Prints the manual page and exits.

=item B<--transfer_ref>=FILE

Filename of a transfer.ref alternative to the default '/etc/transfer.ref'.

=back

=head1 DESCRIPTION

B<cvorg_install.pl> parses a B<transfer.ref> file (taken by default from '/etc/transfer.ref')
and install it using the information gets from this file.
Interrupt vectors, which are available in the transfer.ref file in the install instructions,
are passed as arguments to the B<CVORG> kernel module.

By default all of B<CVORG> devices in the transfer.ref are processed.
Each irq provided is assigned to each device in the
same order as it appears in the transfer.ref file.

=head1 AUTHOR

Written by Samuel Iglesias Gonsalvez <siglesia@cern.ch>

=cut
