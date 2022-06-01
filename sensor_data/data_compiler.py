# -*- coding: utf-8 -*-
"""
Created on Tue May 17 18:13:32 2022

@author: Jasmine Moreira
"""

import csv

data = []

with open(r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\sensor_data\raw_idle.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=' ')
    for row in csv_reader:         
        data.append(row + ['0']) 
        
with open(r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\sensor_data\raw_movement.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=' ')
    for row in csv_reader:             
        data.append(row + ['1'])        
 
 
with open(r'C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\sensor_data\raw_all.csv', 'w', encoding='UTF8', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["AccX","Accy","AccZ",
                     "GyroX","GyroY","GyroZ",
                     "MAGyroX","MAGyroY","MAGyroZ",
                     "Gyro_minus_MAGyroX","Gyro_minus_MAGyroY","Gyro_minus_MAGyroZ",
                     "AADX","AADY","AADZ","AADM",
                     "Categorie"
                     ])
    writer.writerows(data)




from micromlgen import port
from sklearn.naive_bayes import GaussianNB
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier

feat = []
target = []
for row in data:
    feat.append([float(row[15])])
    target.append(int(row[16]))

#classifier
nb_clf = GaussianNB().fit(feat, target)
dt_clf = DecisionTreeClassifier(max_depth=100, min_samples_leaf=2).fit(feat, target)
rf_clf = RandomForestClassifier(n_estimators=2, max_depth=100, min_samples_leaf=5).fit(feat, target)

#check if it's compatible to Orange analysis
acc = 0
for f,t in zip(feat,target):
    if rf_clf.predict([f]) == t:
        acc = acc+1
acc = acc/len(feat)
print("accuracy: "+str(acc))

print(port(rf_clf))



