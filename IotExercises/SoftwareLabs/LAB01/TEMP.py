#!/usr/bin/env python3
# -*- coding: utf-8 -*-
def convert_Temperature(a1, m1, m2):
     if m1=='C' and m2=='K':
         finalUnit = round(int(a1)+273.15, 2)
     elif m1=='K' and m2=='C':
         finalUnit = round(int(a1)-273.15, 2)
     elif m1=='C' and m2=='F':
         finalUnit = round((int(a1)*(9/5))+32, 2)
     elif m1=='F' and m2=='C':
         finalUnit = round((int(a1)-32)*(5/9), 2)
     elif m1=='K' and m2=='F':
         finalUnit = round(((int(a1)-273.15)*(9/5))+32, 2)
     elif m1=='F' and m2=='K':
         finalUnit = round(((int(a1)-32)*(5/9))+273.15, 2) 
     return finalUnit