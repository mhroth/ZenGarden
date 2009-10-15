#!/usr/bin/python

import os

for root, dirs, files in os.walk('./'):
 for name in files:
   if name.endswith(".cpp"):
     filename = os.path.join(root, name)
     print filename + " \\"
