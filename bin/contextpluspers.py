#! /bin/python
# -*- coding: utf-8 -*-

import os

list_context = open('context.txt').readlines()
list_pers = open('PersPram.csv').readlines()

dict_context = {}
dict_pers = {}

directory = ''

for line in list_context:
	tmp0 = line.strip().split(',')
	tmp = tmp0[0].split('t_')
	line = tmp[0]+tmp[1]+','+tmp0[1]
	directory = tmp[0].replace('transed_','')
	dict_context[tmp[1]] = tmp0[1]
for line in list_pers:
	tmp = line.strip().split(',')
	dict_pers[tmp[0].split('\\')[-1]] = tmp[1] + ',' + tmp[2] +','+tmp[3]+','+tmp[4]

fout = open('contextWithPers.txt','w')

for k in dict_context:
	fout.write(directory+k +','+ dict_context[k]+','+dict_pers[k]+'\n')