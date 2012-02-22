#!/usr/bin/perl
#
# sis3300_install.pl - install a sis3300 device
#
# documentation:
# sis3300_install.pl --help or sis3300_install.pl --man
#

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

my $transfer_ref = '/etc/transfer.ref';
my $man = 0;
my $help = 0;
my @AoH;
my @keys = ('ln', 'mln', 'bus', 'mtno', 'module-type', 'lu', 'W1', 'AM1',
	    'DPsz1', 'basaddr1', 'range1', 'W2', 'AM2', 'DPsz2', 'basaddr2',
	    'range2', 'testoff', 'sz', 'sl', 'ss', 'L', 'Vect');

my %base_addrs;


# allow single-character options
Getopt::Long::Configure("bundling_override");

GetOptions(
    "man"		=> \$man,
    "transfer_ref=s"	=> \$transfer_ref,
    ) or pod2usage(2);
pod2usage(1) if $help;
pod2usage(-exitstatus => 0, -verbose => 2) if $man;

open(INPUT, "<$transfer_ref") or die ("$transfer_ref not found\n");
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

# don't add modules with index values that we have already picked up
    if ($base_addrs{"$href->{'lu'}"}) {
	#print "warning: $href->{'module-type'}.$href->{'lu'} has an already taken index number and won't be installed\n";
    } else {
	push @AoH, $href;
	$base_addrs{$href->{'lu'}} = $href->{'basaddr1'};
    }
}
close(INPUT);

# If we found at least a module, then vmodttl.ko should be installed
# But if the vmodttl module is already there, we won't install it again

if (@AoH) {
    if (!module_is_loaded('sis33')) {
	sis33_install();
    }
} else {
    die "No SIS3300 modules found in $transfer_ref.\n";
}

if (@AoH) {
    if (!module_is_loaded('sis3300')) {
	sis3300_install(\@AoH);
    }
} else {
    die "No SIS3300 modules found in $transfer_ref.\n";
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

sub sis33_install {
    my $insmod = "insmod sis33.ko";

    system($insmod) == 0 or die("$insmod failed");
}

sub sis3300_install {
    my $AoHref = shift;
    my $index_parm;
    my $irq;
    my $level;
    my $base_parm;

  ENTRY: foreach my $href (@{$AoHref}) {
      next ENTRY if $href->{'module-type'} !~ m/SIS3300/;

     if (defined $index_parm) {
	  $index_parm = $index_parm.','.$href->{'lu'};
	  $base_parm = $base_parm.',0x'.$href->{'basaddr1'};
	  $irq = $irq.','.$href->{'Vect'};
	  $level = $level.','.$href->{'L'};
      } else {
	  $index_parm = $href->{'lu'};
	  $base_parm = '0x'.$href->{'basaddr1'};
	  $irq = $href->{'Vect'};
	  $level = $href->{'L'};
      }
  }
    if (defined $index_parm) {
	my $insmod = "insmod sis3300.ko index=$index_parm base=$base_parm vector=$irq level=$level";
	system($insmod) == 0 or die("$insmod failed");
        print $insmod, "\n";
    }
}
__END__

=head1 NAME

sis3300_install.pl - Install the SIS3300 kernel module using transfer.ref file.

=head1 SYNOPSIS

sis3300_install.pl [OPTIONS]

sis3300_install.pl - Install the SIS3300 kernel module using B<transfer.ref> file.

=head1 OPTIONS

=over 8

=item B<--man>

Prints the manual page and exits.

=item B<--transfer_ref>=FILE

Filename of a transfer.ref alternative to the default '/etc/transfer.ref'.

=back

=head1 DESCRIPTION

B<sis3300_install.pl> parses a B<transfer.ref> file (taken by default from '/etc/transfer.ref')
and install it using the information gets from this file.
Interrupt vectors, which are available in the transfer.ref file in the install instructions,
are passed as arguments to the B<SIS3300> kernel module.

=head1 AUTHOR

Written by Samuel Iglesias Gonsalvez <siglesia@cern.ch>

=cut
