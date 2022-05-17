# -*- coding: utf-8 -*-
"""
Created on Tue May 17 18:13:32 2022

@author: jasmi
"""

import csv

xi = []
yi = []
zi = []
mi = []
ti = []

with open(r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\sensor_data\aad_raw_idle.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:             
        xi.append(row[0])
        yi.append(row[1])
        zi.append(row[2])
        mi.append(row[3])
        ti.append(0)
                
xm = []
ym = []
zm = []
mm = []
tm = []        
with open(r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\sensor_data\aad_raw_movement.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:             
        xm.append(row[0])
        ym.append(row[1])
        zm.append(row[2])
        mm.append(row[3])
        tm.append(1)


data = []
for i in range(0,len(xi)):
    data.append([xi[i], yi[i], zi[i], mi[i], ti[i]])
    
for i in range(0,len(xm)):
    data.append([xm[i], ym[i], zm[i], mm[i], tm[i]])    
 
with open(r'C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\sensor_data\aad_raw_all.csv', 'w', encoding='UTF8', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["x","y","z","m","t"])
    writer.writerows(data)
