#!/usr/bin/perl
#
# vmodio_install.pl - install a vmodio device
#
# documentation:
# vmodio_install.pl --help or vmodio_install.pl --man
#

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

my @irq_vectors;
my @cmd_line_luns;
my $transfer_ref = '/etc/transfer.ref';
my $man = 0;
my $help = 0;
my @AoH;
my @keys = ('ln', 'mln', 'bus', 'mtno', 'module-type', 'lu', 'W1', 'AM1',
	    'DPsz1', 'basaddr1', 'range1', 'W2', 'AM2', 'DPsz2', 'basaddr2',
	    'range2', 'testoff', 'sz', 'sl', 'ss');

my %base_addrs;


# allow single-character options
Getopt::Long::Configure("bundling_override");

GetOptions(
    "man"		=> \$man,
    "lun|U=s"		=> \@cmd_line_luns,
    "irqv|V=s"		=> \@irq_vectors,
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

# add the interrupt vectors and levels
my $i;
for($i = 0; $i < @cmd_line_luns; $i++) {
    foreach my $val (@AoH){
	if($val->{'lu'} == $cmd_line_luns[$i]){
	    $val->{'irqv'} = $irq_vectors[$i];
	}
    }
 }

# If we found at least a module, then vmodttl.ko should be installed
# But if the vmodttl module is already there, we won't install it again
if (@AoH) {
    if (!module_is_loaded('vmodio')) {
	vmodio_install(\@AoH);	
    }
} else {
    die "No VMODIO modules found in $transfer_ref.\n";
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

sub vmodio_install {
    my $AoHref = shift;
    my $index_parm;
    my $irq;
    my $base_parm;

  ENTRY: foreach my $href (@{$AoHref}) {
      next ENTRY if $href->{'module-type'} !~ m/NULLDATA/;

     if (defined $index_parm) {
	  $index_parm = $index_parm.','.$href->{'lu'};
	  $base_parm = $base_parm.',0x'.$href->{'basaddr1'};
	  $irq = $irq.','.$href->{'irqv'};
      } else {
	  $index_parm = $href->{'lu'};
	  $base_parm = '0x'.$href->{'basaddr1'};
	  $irq = $href->{'irqv'};
      }
  }
    if (defined $index_parm) {
	my $insmod = "insmod vmodio.ko lun=$index_parm base_address=$base_parm irq=$irq";
	system($insmod) == 0 or die("$insmod failed");
        print $insmod, "\n";
    }
}
__END__

=head1 NAME

vmodio_install.pl - Install the VMODIO kernel module using transfer.ref file.

=head1 SYNOPSIS

vmodio_install.pl [OPTIONS] 

vmodio_install.pl - Install the VMODIO kernel module using B<transfer.ref> file.

=head1 OPTIONS

=over 8

=item B<--irqv -V>

Interrupt Vector for each device.
Both multiple values and comma-separated lists of values are accepted.

=item B<--lun -l>

Devices to be installed given by their logical unit numbers.
Both multiple values and comma-separated lists of values are accepted.
When missing, all the logical unit numbers found in the transfer.ref
file are installed.

=item B<--man>

Prints the manual page and exits.

=item B<--transfer_ref>=FILE

Filename of a transfer.ref alternative to the default '/etc/transfer.ref'.

=back

=head1 DESCRIPTION

B<vmodio_install.pl> parses a B<transfer.ref> file (taken by default from '/etc/transfer.ref')
and install it using the information gets from this file. 
Interrupt vectors, which are available in the transfer.ref file in the install instructions, 
are passed as arguments to the B<VMODIO> kernel module.

By default all of B<VMODIO>'s devices in the transfer.ref are processed.
Each irq provided is assigned to each device in the
same order as it appears in the transfer.ref file.

=head1 AUTHOR

Written by Samuel I. Gonsalvez

=cut
