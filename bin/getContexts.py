#! /bin/python
# -*- coding: utf-8 -*-

import os
exename = 'transedImage2context.exe'
rootDir = os.path.join(os.getcwd(), 'transed_images')
print rootDir
list_dirs = os.walk(rootDir)
for root, dirs, files in list_dirs: 
	#for d in dirs: 
		#print os.path.join(root, d)      
	for f in files: 
		print os.path.join(root, f)
		image = os.path.join(root, f)
		os.system(exename + ' ' + image)