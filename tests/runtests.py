# -*- Mode: Python -*-

import os
import glob

import unittest


# Load tests.
if 'TEST_NAMES' in os.environ:
	names = os.environ['TEST_NAMES'].split()
elif 'TEST_FILES' in os.environ:
	names = []
	for filename in os.environ['TEST_FILES'].split():
		names.append(filename[:-3])
else:
	names = []
	for filename in glob.iglob("test_*.py"):
		names.append(filename[:-3])

loader = unittest.TestLoader()
suite = loader.loadTestsFromNames(names)


# Run tests.
runner = unittest.TextTestRunner(verbosity=2)
runner.run(suite)

