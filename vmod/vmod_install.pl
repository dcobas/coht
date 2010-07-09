#!/usr/bin/perl
# Install vmodttl modules from a transfer.ref file.
# Based on install script of sis33 modules created by Emilio G. Cota
#
# first the transfer.ref file is parsed and then, after some sanity checks,
# insmod is called with the parameters extracted from transfer.ref.
#
# call with 'script.pl --type 00' to install vmodttl modules only.
# by default it tries to install all the modules it finds.

use strict;
use warnings;

use Getopt::Long;
use Pod::Usage;

my $device;
my $driver;
my $help;
my $man;
my $path = '/etc/transfer.ref';
my @AoH;
my @keys = ('ln', 'mln', 'bus', 'mtno', 'module-type', 'lu', 'W1', 'AM1',
	    'DPsz1', 'basaddr1', 'range1', 'W2', 'AM2', 'DPsz2', 'basaddr2',
	    'range2', 'testoff', 'sz', 'sl', 'ss');

my %base_addrs;

my $carrier_register = "modulbus_register";
my $insmod_register = "insmod $carrier_register.ko";

GetOptions(
	   "help|?|h"=> \$help,
	   "device=s"=> \$device,
	   "man"=> \$man,
	   "transfer_ref=s"=> \$path,
	   ) or pod2usage(2);

pod2usage(-exitstatus => 0, -verbose => 2) if $man;

if (@ARGV == 0) {
    print STDERR "Missing driver name\n";
    pod2usage(2);
} else {
    $driver = $ARGV[0];
}

if (!defined $device) {
    $device = $driver;
}

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

# If we found at least a module, then vmodttl.ko should be installed
# But if the vmodttl module is already there, we won't install it again
if (@AoH) {
    if (!module_is_loaded($carrier_register)) {
	system($insmod_register) == 0 or die("$insmod_register failed");
    }
    if (!module_is_loaded($driver)) {
	vmod_install(\@AoH);
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

sub vmod_install {
    my $AoHref = shift;
    my $index_parm;
    my $carrier_lun;
    my $carrier_number;
    my $slot;
    my $carrier_name;
    my $carrier;

    my $hrefProc;

  ENTRY: foreach my $href (@{$AoHref}) {
      next ENTRY if $href->{'module-type'} !~ m/$device/;

# We search the carrier of the installed VMOD-like board.
# To know it, we use the slot and subslot fields.
    CARRIER_ENTRY: foreach my $href_carrier (@{$AoHref}) {
	next CARRIER_ENTRY if $href_carrier->{'module-type'} =~ m/$device/;

	if(($href_carrier->{'sl'} == $href->{'sl'}) &&
	   ($href_carrier->{'ss'} == -1)){
	    $carrier_name = lc $href_carrier->{'module-type'};
	    $carrier_lun = $href_carrier->{'lu'};
	    last if (1);
	}
      }
      if (defined $index_parm) {
	  $index_parm = $index_parm.','.$href->{'lu'};
	  $carrier = $carrier.','.$carrier_name;
	  $carrier_number = $carrier_number.','.$carrier_lun;
	  $slot = $slot.','.$href->{'ss'};
      } else {
	  $index_parm = $href->{'lu'};
	  $carrier = $carrier_name;
	  $carrier_number = $carrier_lun;
	  $slot = $href->{'ss'};
      }
      $carrier_name = "";
      $carrier_lun = "-1";

  }
    if (defined $index_parm) {
	my $driver_name = $driver.".ko";
	my $insmod = "insmod $driver_name lun=$index_parm carrier=$carrier carrier_number=$carrier_number slot=$slot";
	print $insmod, "\n";
	system($insmod) == 0 or die("$insmod failed");

	# Create the nodes in /dev for each lun.
	my $mayor = -1;
	system("grep $driver /proc/devices > /tmp/tmp_$driver") == 0 or die("grep $driver in /proc/devices failed");

	open(INPUT, "</tmp/tmp_$driver") or die ("/tmp/tmp_$driver not found");
	LINE: while (<INPUT>) {
	    chomp;
   	    my @values = split(/\s+/, $_);
	    $mayor = $values[0];
	}
	close(INPUT);
	system("rm -f /tmp/tmp_$driver");

  	ENTRY: foreach my $hrefMknod (@{$AoHref}) {
      		next ENTRY if $hrefMknod->{'module-type'} !~ m/$device/;

	 	my $mknod = "mknod /dev/$driver.$hrefMknod->{'lu'} c $mayor $hrefMknod->{'lu'}";
	 	system($mknod) == 0 or die("$mknod failed");
		#print $mknod, "\n";
	}
    }
}

__END__

=head1 NAME

vmod_install.pl - Install the VMOD-like kernel module using transfer.ref file.

=head1 SYNOPSIS

vmod_install.pl [OPTIONS] driver

vmod_install.pl - Install the VMOD-like kernel module using B<transfer.ref> file.

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

B<vmod_install.pl> parses a B<transfer.ref> file (taken by default from '/etc/transfer.ref')
and install it using the information gets from this file.
Interrupt vectors, which are available in the transfer.ref file in the install instructions,
are passed as arguments to the B<VMOD-like> kernel module.

By default all of B<VMOD-like> devices in the transfer.ref are processed.
Each irq provided is assigned to each device in the
same order as it appears in the transfer.ref file.

=head1 AUTHOR

Written by Samuel I. Gonsalvez

=cut
