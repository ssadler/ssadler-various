#!/usr/bin/env python

import sys, optparse, time

__version__ = (0, 3)

parser = optparse.OptionParser(version="%prog "+'.'.join(map(str, __version__)))
parser.add_option('-f', "--frequency", dest="frequency", default=1.0, type="float",
                  help="Frequency in data writes per second")
parser.add_option('-n', "--num", dest='num', default=1, type="int", help="number of lines to write at a time")
parser.add_option('-o', '--offset', dest="offset", default=0, type="int", help="line number to start at")
parser.add_option('-l', '--len', dest="length", default=None, type="int", help="max number of lines to write")

def main():
    opts, args = parser.parse_args()
    if len(args) != 0:
        parser.error("incorrect number of arguments")
    
    source = enumerate(sys.stdin)
    lineno = -1
    
    try:
        # skip to offset
        while lineno < opts.offset-1:
            lineno, line = source.next()
        
        if opts.length != None:
            max_length = opts.length + opts.offset
        else:
            max_length = None
        
        sleeptime = 1 / float(opts.frequency)

        # start writing lines
        while True:
            start = time.time()
            for i in xrange(opts.num):
                # break if at max length
                if max_length != None:
                    if lineno+1 >= max_length:
                        raise StopIteration()
                lineno, line = source.next()
                print >>sys.stderr, lineno
                sys.stderr.flush()
                sys.stdout.write(line)
                sys.stdout.flush()
            op_time = time.time() - start
            op_diff = sleeptime - op_time
            if op_diff > 0:
                time.sleep(op_diff)
    except StopIteration:
        pass


if __name__ == '__main__':
    main()
