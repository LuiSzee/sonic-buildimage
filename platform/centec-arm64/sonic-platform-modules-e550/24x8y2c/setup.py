#!/usr/bin/env python

import os
import sys
from setuptools import setup
os.listdir

setup(
   name='24x8y2c',
   version='1.1',
   description='Module to initialize centec e550-24x8y2c platforms',
   
   packages=['24x8y2c'],
   package_dir={'24x8y2c': '24x8y2c/classes'},
)

