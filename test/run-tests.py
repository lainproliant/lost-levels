#!/bin/env python

#
# run-tests.py: A basic test harness for executable tests.
#
# Runs all tests in the current directory, prints and returns the total
# number of tests which failed.
#
# Author: Lain Supe (lainproliant)
# Date: Fri October 10 2014
#

import glob
import subprocess
import sys
import os

def main(argv):
    tests_failed = 0
    modules_failed = 0

    skip_tests = set((os.environ.get("SKIP_TESTS") or "").split(","))

    if len(skip_tests) > 0:
        print("Skipping tests: %s" % repr(skip_tests))

    tests = glob.glob("./*.test")
    for test in [test for test in tests if test not in skip_tests]:
        num_failed = subprocess.call(test)
        if num_failed > 0:
            modules_failed += 1
            tests_failed += num_failed

    print("===== SUMMARY =====")
    print("    %d modules PASSED." % (len(tests) - modules_failed))
    if modules_failed > 0:
        print("    %d modules FAILED, %d overall tests FAILED." % (
            modules_failed, tests_failed));

    sys.exit(tests_failed)

if __name__ == "__main__":
    main(sys.argv[1:])

