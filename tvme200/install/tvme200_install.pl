#!/usr/bin/perl
#
# tvme200_install.pl - install a tvme200 device
#
# documentation:
# tvme200_install.pl --help or tvme200_install.pl --man
#

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

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
    push @AoH, $href;
    $base_addrs{$href->{'lu'}} = $href->{'basaddr1'};
}
close(INPUT);

# If we found at least a module, then tvme200.ko should be installed
# But if the tvme200 module is already there, we won't install it again
if (@AoH) {
   if (!module_is_loaded('tvme200')) {
	tvme200_install(\@AoH);	
    }
} else {
    die "No TVME200 modules found in $transfer_ref.\n";
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

sub tvme200_install {
    my $AoHref = shift;
    my $index_parm;
    my $mod_address_ioid;
    my $mod_address_mem;
    my $width_ioid;
    my $width_mem;
    my $size_ioid;
    my $size_mem;
    my $base_ioid;
    my $base_mem;

  ENTRY: foreach my $href (@{$AoHref}) {
      next ENTRY if $href->{'module-type'} !~ m/TVME200/;

     if (defined $index_parm) {
	  $index_parm = $index_parm.','.$href->{'lu'};
	  $base_ioid = $base_ioid.',0x'.$href->{'basaddr1'};
	  $base_mem = $base_mem.',0x'.$href->{'basaddr2'};
	  $mod_address_ioid = $mod_address_ioid.',0x29';
	  $mod_address_mem = $mod_address_mem.',0x39';
	  $width_ioid = $width_ioid.',16';
	  $width_mem = $width_mem.',16';
	  $size_ioid = $size_ioid.',0x0400';
	  $size_mem = $size_mem.',0x20000';
      } else {
	  $index_parm = $href->{'lu'};
	  $base_ioid = '0x'.$href->{'basaddr1'};
	  $base_mem = '0x'.$href->{'basaddr2'};
	  $mod_address_ioid = '0x29';
	  $mod_address_mem = '0x39';
	  $width_ioid = '16';
	  $width_mem = '16';
	  $size_ioid = '0x0400';
	  $size_mem = '0x20000';
      }
  }
    if (defined $index_parm) {
	my $insmod = "insmod tvme200.ko lun=$index_parm mod_address_ioid=$mod_address_ioid data_width_ioid=$width_ioid wind_size_ioid=$width_ioid base_address_ioid=$base_ioid mod_address_mem=$mod_address_mem data_width_mem=$width_mem wind_size_mem=$size_mem base_address_mem=$base_mem";
	system($insmod) == 0 or die("$insmod failed");
        print $insmod, "\n";
    }
}
__END__

=head1 NAME

tvme200_install.pl - Install the TVME200 kernel module using transfer.ref file.

=head1 SYNOPSIS

tvme200_install.pl [OPTIONS] 

tvme200_install.pl - Install the TVME200 kernel module using B<transfer.ref> file.

=head1 OPTIONS

=over 8

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

B<tvme200_install.pl> parses a B<transfer.ref> file (taken by default from '/etc/transfer.ref')
and install it using the information gets from this file. 

By default all of B<TVME200>'s devices in the transfer.ref are processed.

=head1 AUTHOR

Written by Samuel I. Gonsalvez <samuel.iglesias.gonsalvez@cern.ch>

=cut
