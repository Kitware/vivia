#!/usr/bin/perl -w

my @td = localtime(time);
my $year = 1900 + $td[5];

my $c_statement = "/*ckwg +5
 * Copyright $year by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */
";

my $sh_statement = "#ckwg +4
# Copyright $year by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
";

my $bat_statement = "::ckwg +4
:: Copyright $year by Kitware, Inc. All Rights Reserved. Please refer to
:: KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
:: Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
";


foreach my $filename ( @ARGV )
{
    print STDERR "Processing $filename\n";
    if( open( my $fd, $filename ) )
    {
        my $file = join("",<$fd>);
        close( $fd );

        my $statement;
        if( $filename =~ /.*\.bat(\.in)?$/i ) {
            $statement = $bat_statement
        } elsif( $filename =~ /.*\.(py|pl|sh)(\.in)?$/i ) {
            $statement = $sh_statement
        } else {
            $statement = $c_statement
        }

        if( $file =~ m:(^|.*\n)((//|/\*|#|\:\:) *ckwg *(\*/)?)(.*):s ) {
            $file = $1.$statement.$5;
        } else {
            print STDERR "$filename does not have a marker\n";
        }
        if( open( $fd, ">$filename" ) ) {
            print $fd $file;
            close( $fd );
        } else {
            print STDERR "Couldn't open $filename for writing\n";
        }

    } else {
        warn( "Couldn't open $filename" );
    }
}
