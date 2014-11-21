#!/bin/env python

import argparse
import ast
import glob
import json
import os
import re
import subprocess
import sys

import pyproj
from osgeo import gdal

#==============================================================================
def parse_wkt(text):
  parts = None
  n = 0
  result = {}
  final = False

  while len(text):
    # Determine if next field is a named object or a value
    xbb = text.find('[')
    xeb = text.find(']')
    xc = text.find(',')
    if xc == 0:
      # End of object
      text = text[1:]
    elif xeb == 0:
      # End of parent object
      return (result, text[1:])
    elif xbb > 0 and (xc < 0 or xbb < xc) and xbb < xeb:
      # Named object; extract value
      parts = text.partition('[')
      data = parse_wkt(parts[2])
      result[parts[0]] = data[0]
      text = data[1]
    else:
      # Value
      if xc > 0 and xc < xeb:
        parts = text.partition(',')
      else:
        parts = text.partition(']')
        final = True

      result[n] = ast.literal_eval(parts[0])
      n += 1
      text = parts[2]

    if final:
      return (result, text)

  return (result, '')

#==============================================================================
def transform_point(x, y, xf, scale):
  return (scale * (xf[0] + (xf[1] * x) + (xf[2] * y)),
          scale * (xf[3] + (xf[4] * x) + (xf[5] * y)))

#==============================================================================
def make_pair(pair_string):
  pair = pair_string.split('=')
  if len(pair) == 1:
    pair += [None]
  return pair

#==============================================================================
def units_for_unit(gcs, meters_per_unit):
  gcs = '<%i>' % gcs

  # Find proj.4 init string for GCS
  init_info_file = file(os.path.join(pyproj.datadir.pyproj_datadir, 'epsg'))
  for line in init_info_file:
    if line.startswith(gcs):
      # Get init key/value pairs
      init_info = dict([make_pair(pair) for pair in line.split(' ')[1:]])

      # Check if init info specifies units
      if '+units' in init_info:
        # Get units for GCS
        units = init_info['+units']

        # Determine GCS units per input unit
        if units == 'm':
          return meters_per_unit
        elif units == 'us-ft':
          return meters_per_unit / 0.3048006096012192

  return 1.0 # Don't know :-(

#==============================================================================
def parse_file(path, out, args):
  # Get short name of input file, without extension
  in_name = os.path.splitext(os.path.basename(path))[0]
  print in_name, ':'

  # Open file with GDAL
  gf = gdal.Open(path)

  # Get projection for file
  proj = parse_wkt(gf.GetProjection())[0]['PROJCS']
  gcs = int(proj['AUTHORITY'][1])
  meters_per_unit = proj['UNIT'][1]
  print '  gcs = %i' % gcs

  # Get geotransform for file
  gsd = -1.0
  xf = gf.GetGeoTransform()
  if xf[2] == 0 and xf[4] == 0:
    gsd = abs(xf[1]) * meters_per_unit * 100.0
    print '  gsd = %.2f cm/px' % gsd

  # Get corner points for file
  w = gf.RasterXSize
  h = gf.RasterYSize
  units_per_unit = units_for_unit(gcs, meters_per_unit)
  ul = transform_point(0, 0, xf, units_per_unit)
  ur = transform_point(w, 0, xf, units_per_unit)
  ll = transform_point(0, h, xf, units_per_unit)
  lr = transform_point(w, h, xf, units_per_unit)

  print '  upper left  = (%16.7f, %16.7f)' % ul
  print '  upper right = (%16.7f, %16.7f)' % ur
  print '  lower left  = (%16.7f, %16.7f)' % ll
  print '  lower right = (%16.7f, %16.7f)' % lr

  # Write tile to output KST
  out.write('"%s/%s.mrj", %.2f, %i,\n' % (args.name, in_name, gsd, gcs))
  out.write('  %.7f, %.7f,\n' % ul)
  out.write('  %.7f, %.7f,\n' % ur)
  out.write('  %.7f, %.7f,\n' % ll)
  out.write('  %.7f, %.7f;\n' % lr)

#==============================================================================
def parse_input(file_or_dir, out, args):
  if os.path.isdir(file_or_dir):
    for pat in ('[tT][iI][fF]', '[tT][iI][fF][fF]', '[jJ][pP]2'):
      for x in glob.glob(os.path.join(file_or_dir, '*.%s' % pat)):
        parse_file(x, out, args)
  else:
    parse_file(file_or_dir, out, args)

#==============================================================================
if __name__ == '__main__':
  # Parse arguments
  parser = argparse.ArgumentParser(description='Context KST generator')
  parser.add_argument('name', metavar='output-name',
                      help='short name of context set')
  parser.add_argument('inputs', metavar='in', nargs='+',
                      help='input image, or directory containing input images')
  args = parser.parse_args()

  out = open('context-%s.kst' % args.name, 'w')
  out.write('# file, GSD, GCS, { easting, northing } ( ul, ur, ll, lr )\n')

  for i in args.inputs:
    parse_input(i, out, args)

## Get options
#name=$1
#gsd=$2
#out="context-$name.kst"
#shift 2

## Get file list
#if [ -d "$1" ]; then
    #oldifs="$IFS"
    #IFS=$'\n'
    #files=( $( find "$1" -iname '*.tif' -o -iname '*.tiff' ) )
    #IFS="$oldifs"
#else
    #files=( "$@" )
#fi

## Write header
#echo '# file, GSD, GCS, { lat, lon } ( ul, ur, ll, lr )' > "$out"

#for f in "${files[@]}"; do
    ## Get listgeo information
    #oldifs="$IFS"
    #IFS=$'\n'
    #info=( $( listgeo "$f" | \
           #sed -n -e '/^PCS/s,^[^=]*= *,,p' \
                  #-e '/^\(Upper\|Lower\)/s,^.*[(] *\([^)]*\)[)],\1,p' ) )
    #IFS="$oldifs"

    ## Parse listgeo information
    #gcs=${info[0]%% (*}

    ## Write tile info
    #b="$(basename "$f" | sed 's,\.tiff\?$,,I')"
    #cat >> "$out" << EOF
#"$name/$b.mrj", $gsd, $gcs,
    #${info[1]%)},
    #${info[3]%)},
    #${info[2]%)},
    #${info[4]%)};
#EOF
#done
