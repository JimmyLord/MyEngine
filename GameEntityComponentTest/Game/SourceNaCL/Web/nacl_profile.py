#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" %prog is a tool for profiling NaCl modules on 64-bit
Windows.

It is a thin wrapper around AMD CodeAnalyst. CodeAnalyst does the
actual profiling, and %prog maps the samples in the NaCl
sandbox to symbols in the nexe files and the IRT (NaCl Integrated
RunTime).

"""

import csv
import optparse # TODO(cstefansen): Use argparse instead.
import os.path
import re
import shutil
import subprocess
import sys


def ParseProfileData(options, profile_data_file, objdump, base):
    # TODO(cstefansen): Separate processing and formatting more cleanly in this
    # function.
    offset = int(options.offset, 16)
    first = True
    numSkipped = 0
    header = None
    total = {}
    out = []

    for row in csv.reader(open(profile_data_file)):
        if 'Timer Samples' == row[0]:
            header = row # This is the header; keep it
        else:
            address = int(row[1], 16)-offset
            amt = int(row[0])
            symbol = objdump.get(address, row[1])

            # Did we find a symbol for this address?
            if symbol != row[1]:
                if options.no_collapse:
                    symbol = '%s+%d' % (symbol, address-base[symbol])
                else:
                    total[symbol] = total.get(symbol, 0) + amt
                    continue
            # Add the sample to the output.
            out.append((row[0], '0x%011x' % (address + offset), symbol))

    # Add collapsed samples.
    for symbol, time in total.iteritems():
        out.append((str(time), '0x%011x' % (base[symbol] + offset), symbol))

    # Add percentages.
    total_samples = sum([int(row[0]) for row in out])
    out = map(lambda row: ('%.1f' % (100*float(row[0])/total_samples),
                           row[0], row[1], row[2]),
              out)

    # Sort the rows, most time first.
    out.sort(key=lambda e: float(e[0]), reverse=True)


    if options.output:
        # Write to a file
        basedir = os.path.dirname(options.output)
        if basedir and not os.path.exists(basedir):
            os.makedirs(basedir)
        fo = open(options.output, 'w')
        print 'Writing NaCl profiling results to %s.' % options.output
    else:
        # Write to the terminal
        fo = sys.stdout

    if options.csv:
        # Add the header
        h1, h2, h3 = header
        out.insert(0, ('Percent', h1, h2, h3))
        csv.writer(
            fo, delimiter=',',
            quoting=csv.QUOTE_MINIMAL,
            lineterminator='\n').writerows(out
                                           if options.top.lower() == 'all'
                                           else out[:int(options.top)+1])
    else:
        # Pretty printing via un-pretty code
        fo.write('\n    % Address       Symbol\n')
        fo.write('----- ------------- ------'
                 '-----------------------------------------------------\n')
        for row in (out if options.top.lower() == 'all'
                    else out[:int(options.top)+1]):
            fo.write('%s %s %s\n' % (format(row[0], '>5'), row[2], row[3][:59]))
            index = 59
            while len(row[3][index + 1:]) > 0:
                fo.write('                    ' + row[3][index:index+59] + '\n')
                index += 59


# Assumes line format: hexoffset junk hexlength symbol
lineparser = re.compile(r'(^[\da-fA-F]+)\s[^\d]+\s([\da-fA-F]+)\s+(.+)$')

def GetObjdump(objdump_file, functions, base):
    data = open(objdump_file).read()
    for line in data.split('\n'):
        # Filter out lines we are not interested in.
        line = line.strip()
        if not line: continue
        if '.text' not in line: continue
        if line == 'SYMBOL TABLE:': continue
        if 'file format elf' in line: continue

        parsed = lineparser.search(line)
        assert parsed is not None, line
        offset = int(parsed.group(1), 16)
        length = int(parsed.group(2), 16)
        symbol = parsed.group(3)

        # Pseudo symbol?
        if length == 0: continue

        # The starting address for this symbol.
        base[symbol] = offset

        # HACK creates a hash table with an entry for every byte in the program.
        # A more elegant approach would be to represent byte ranges for each
        # function, but the lookup would be more complicated.
        for i in range(offset, offset+length):
            assert i not in functions, functions[i]
            functions[i] = symbol

    return functions, base


def MakeCommandLineParser():
    parser = optparse.OptionParser(description='%prog is a tool for profiling '
                                   'NaCl modules on 64-bit Windows.',
                                   usage='%prog [-h] [options]')
    parser.add_option('--command_line', dest='command_line', action='store',
                      type='string', default=None,
                      help='Optional command to run before profiling (e.g., '
                      'Chrome)')
    parser.add_option('--nexe_file', dest='nexe_file', action='store',
                      type='string', default=None,
                      help='Path to the unstripped .nexe file being run')
    parser.add_option('--irt_file', dest='irt_file', action='store',
                      type='string', default=None,
                      help='Path to the unstripped IRT being used')
    parser.add_option('--csv', dest='csv', action='store_true',
                      default=False,
                      help='Output results (NaCl only) in comma-separated '
                      'format')
    parser.add_option('--out', dest='output', action='store',
                      type='string', default=None,
                      help='Output file.')
    parser.add_option('--offset', dest='offset', action='store',
                      type='string', default='0xc00000000',
                      help='Sandbox memory offset for NaCl.')
    parser.add_option('--delay', dest='delay', action='store',
                      type='int', default='0',
                      help='Delay in seconds before starting profiler.')
    parser.add_option('--top', dest='top', action='store',
                      type='string', default='30',
                      help='Number of lines to print (default 30) or --top=all'
                      ' for everything.')
    parser.add_option('--no_collapse', dest='no_collapse', action='store_true',
                      default=False,
                      help='Don\'t collapse profiler samples into a single '
                      'sample for each function.')
    parser.add_option('--run_codeanalyst', dest='run_codeanalyst',
                      action='store_true', default=False,
                      help='Launch CodeAnalyst to see system-wide non-NaCl '
                      'data when done.')
    parser.add_option('--no_system_data', dest='no_system_data',
                      action='store_true', default=False,
                      help='Don\'t print system-wide module/process data. If '
                      'you don\'t print the data, you can still see it by '
                      'opening the .caw file in CodeAnalyst.')
    return parser


def Main():
    # TODO(cstefansen) This is long and does not separate I/O from processing
    # very cleanly. Fix that and factor out into smaller functions.

    # Set-up and pre-flight checks
    parser = MakeCommandLineParser()
    options, args = parser.parse_args()
    if options.nexe_file is None:
        parser.error('--nexe_file is required.')
    if args:
        parser.error('Invalid argument given. Use --help for instructions.')
    basename = os.path.splitext(
        os.path.basename(options.nexe_file))[0]

    print 'Performing pre-flight checks.'
    for c in ['caprofile /v', 'cadataanalyze /v', 'careport /?',
              'x86_64-nacl-objdump -v']:
        try:
            with open(os.devnull, 'w') as dn:
                p = subprocess.Popen(c, stdout=dn)
        except Exception as e:
            sys.exit('Could not launch %s.\nPlease make sure '
                     'AMD CodeAnalyst and the Native Client SDK are installed '
                     'and\nthat the command line tools are in the path.\n'
                     'Error %i: %s' % (c, e[0], e[1]))


    # Get objdump
    print 'Getting symbols from nexe.'
    objdump_file = basename + '.objdump'
    os.system('x86_64-nacl-objdump -t -C %s > %s'
              % (options.nexe_file, objdump_file))
    objdump, base = GetObjdump(objdump_file, {}, {})


    # Get IRT objdump if requested
    if options.irt_file:
        print 'Getting symbols from IRT.'
        irt_objdump_file = os.path.splitext(
            os.path.basename(options.irt_file))[0 ] + '.objdump'
        os.system('x86_64-nacl-objdump -t -C %s > %s'
                  % (options.irt_file, irt_objdump_file))
        objdump, base = GetObjdump(irt_objdump_file, objdump, base)


    # Clean up previous profiling data
    profile_dir = '%s.tbp.dir/' % basename
    if os.path.exists(profile_dir):
        # (Acceptable) race condition: the directory could have been
        # deleted between checking and attempting removal
        try:
            shutil.rmtree(profile_dir)
        except Exception:
            sys.exit('Could not remove previous profiling data. Is '
                     'CodeAnalyst running?')


    # Set up CodeAnalyst workspace
    print 'Setting up CodeAnalyst workspace.'
    caw_file = basename + '.caw'
    shutil.copyfile(
        os.path.splitext(sys.argv[0])[0] + '.caw_template',
        caw_file)


    # Profile!
    print 'Running profiler for 20 seconds.'
    if options.command_line is None:
        os.system('caprofile.exe /s /sd %u /d 20 /o %s'
                  % (options.delay, basename))
    else:
        os.system('caprofile.exe /s /sd %u /d 20 /o %s /b /l %s'
                  % (options.delay, basename, options.command_line))

    os.system('cadataanalyze.exe /i %s.prd /o %s /a %s'
              % (basename, basename, caw_file))
    summary_file = '%s.tbp.dir/%s.tbp' % (basename, basename)


    # Print system-wide data if requested
    if not options.no_system_data:
        os.system('careport.exe /P /%% /n 20 /q %s' % summary_file)
        os.system('careport.exe /M /%% /n 20 /q %s' % summary_file)
        print


    # Find probable NaCl process id(s) and extract data for them
    profile_summary = open(summary_file, 'r')
    process_ids = set({})
    line = profile_summary.readline()
    while line:
        # TODO(cstefansen): make this work on non-Win64
        if line.find('nacl64.exe') != -1:
            process_ids.add(line.split(',')[0])
        line = profile_summary.readline()

    if len(process_ids) == 0:
        sys.exit('Profile data contains no Chrome NaCl processes (no '
                 'samples collected in nacl64.exe). Make sure your NaCl '
                 'application is running and in the foreground during '
                 'profiling')
    print 'Probable nexe process ids: ', list(process_ids)

    profile_data_file = basename + '.cacsv'
    try:
        os.remove(profile_data_file)
    except Exception:
        sys.exc_clear()

    for pid in process_ids:
        os.system('careport.exe /o CSV /q /F "unknown module pid (%s)" '
                  '%s >> %s' % (pid, summary_file, profile_data_file))


    # Map profile data to symbols from objdump(s)
    ParseProfileData(options, profile_data_file, objdump, base)

    if options.run_codeanalyst:
        os.system('codeanalyst.exe %s' % caw_file)

if __name__ == '__main__':
  Main()
