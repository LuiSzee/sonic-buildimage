#!/usr/bin/env python

import os
import sys
from setuptools import setup
os.listdir

setup(
   name='24t16y',
   version='1.1',
   description='Module to initialize centec e550-24t16y platforms',
   
   packages=['24t16y'],
   package_dir={'24t16y': '24t16y/classes'},
)

