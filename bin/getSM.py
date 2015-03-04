#! /bin/python
# -*- coding: utf-8 -*-

import os

mylist = open('contextWithPers.txt').readlines()

exename = 'context2newSM.exe'

for line in mylist:
	tmp = line.strip().split(',')
	imagefile = tmp[0]
	context = tmp[1]
	L0 = tmp[2]
	L1 = tmp[3]
	R0 = tmp[4]
	R1 = tmp[5]
	print imagefile
	os.system(exename + ' ' + imagefile + ' ' + context + ' ' + L0 + ' ' + L1 +' '+R0+' '+R1)
	os.rename("sm.jpg","result_images/sm_"+imagefile.split('\\')[-1])
	os.rename("context.jpg","result_images/context_"+imagefile.split('\\')[-1])
	os.rename("dst.jpg","result_images/dst_"+imagefile.split('\\')[-1])