# -*- coding: utf-8 -*-
"""
Created on Mon May  2 09:52:52 2022

@author: jasmi
"""

import random
import math
import numpy as np
import tensorflow as tf
from tensorflow.keras import layers
from tensorflow.keras.utils import to_categorical
import matplotlib.pyplot as plt
from matplotlib import colors


# radius of the circle
circle_r = 1
# center of the circle (x, y)
circle_x = 5
circle_y = 7
nsamples = 300
train_percentage = 0.6
val_percentage = 0.2    # 20% from training
test_percentage = 0.2   # 20% for test

square_s = 3 * circle_r 

def getPoint():
    # random angle
    alpha = 2 * math.pi * random.random()
    # random radius
    r = circle_r * random.random()
    # calculating coordinates
    x = r * math.cos(alpha) + circle_x
    y = r * math.sin(alpha) + circle_y
    return(x,y)

def getOuterPoint(x):
    xmin = circle_x - circle_r
    xmax = circle_x + circle_r
    y = circle_y + (square_s * (1/2 - random.random()))
    if x < xmin or x > xmax:
        return x,y
    #return 4,5
    #max and min y
    b = math.sqrt(circle_r**2. - (x-circle_x)**2)
    ymin = circle_y - b  
    ymax = circle_y + b    
    while True:
        y = circle_y + (square_s * (1/2 - random.random()))
        if y > ymax or y < ymin:
            break 
    return x,y

X = []
Y = []
L = []
for i in range(0,int(nsamples/2)):
    x,y = getPoint()
    X.append(x)
    Y.append(y)
    L.append(1)
    x,y = getOuterPoint(circle_x + (square_s * (1/2 - random.random())))
    X.append(x)
    Y.append(y)
    L.append(0)

plt.figure(figsize=(8,8))
plt.scatter(X, Y, c=L, cmap=colors.ListedColormap(['red','blue']))
plt.show()

#simple normalization -> save mean and stddev to normalize new inputs
x_mean = np.mean(X)
y_mean = np.mean(Y)

#x_stdd = np.std(X)
#y_stdd = np.std(Y)
#X = (X-x_mean)/x_stdd
#Y = (Y-y_mean)/y_stdd

X = (X-x_mean)
Y = (Y-y_mean)
X = X/np.max([-1*np.min(X),np.max(X)])
Y = Y/np.max([-1*np.min(Y),np.max(Y)])


data = []
for i in range(0,len(X)):
    data.append([i, X[i], Y[i], L[i]])
 
import csv
with open(r'C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\model_camparison\circular_data_raw.csv', 'w', encoding='UTF8', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["index","x","y","in"])
    writer.writerows(data)


data_train = []
while len(data_train) < nsamples*train_percentage:
    i = int(random.random()*nsamples)
    if(data[i] not in data_train):
        data_train.append(data[i])

data_validation = []
while len(data_validation) < nsamples*val_percentage:
    i = int(random.random()*nsamples)
    if(data[i] not in data_train and data[i] not in data_validation):
        data_validation.append(data[i])
        
data_test = []
while len(data_test) < nsamples*test_percentage:
    i = int(random.random()*nsamples)
    if(data[i] not in data_train and data[i] not in data_validation and data[i] not in data_test):
        data_test.append(data[i])       

train_values = [ [i[1],i[2]] for i in data_train ]
train_values = np.array(train_values, dtype='f')
train_labels = [ i[3] for i in data_train ]
train_labels = to_categorical(train_labels)

plt.figure(figsize=(8,8))
plt.scatter(train_values[:,[0]], train_values[:,[1]], c=train_labels[:,[1]], cmap=colors.ListedColormap(['red','blue']))
plt.show()

val_values = [ [i[1],i[2]] for i in data_validation ]
val_values = np.array(val_values, dtype='f')
val_labels = [ i[3] for i in data_validation ]
val_labels = to_categorical(val_labels)

plt.figure(figsize=(8,8))
plt.scatter(val_values[:,[0]], val_values[:,[1]], c=val_labels[:,[1]], cmap=colors.ListedColormap(['red','blue']))
plt.show()

test_values = [ [i[1],i[2]] for i in data_test ]
test_values = np.array(test_values, dtype='f')
test_labels = [ i[3] for i in data_test ]
test_labels = to_categorical(test_labels)

plt.figure(figsize=(8,8))
plt.scatter(test_values[:,[0]], test_values[:,[1]], c=test_labels[:,[1]], cmap=colors.ListedColormap(['red','blue']))
plt.show()



# Create a model
model = tf.keras.Sequential()
model.add(layers.Dense(50,activation='relu', input_shape=(2,)))
#model.add(layers.Dense(10,activation='relu'))
model.add(layers.Dense(2, activation='softmax'))
# View model
model.summary()

# Add optimizer, loss function, and metrics to model and compile it
model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])

# Train model
history = model.fit(train_values,
                    train_labels,
                    epochs=400,
                    batch_size=50,
                    validation_data=(val_values, val_labels))

# Plot the training history
loss = history.history['loss']
val_loss = history.history['val_loss']

epochs = range(1, len(loss) + 1)

plt.plot(epochs, loss, 'bo', label='Training loss')
plt.plot(epochs, val_loss, 'b', label='Validation loss')
plt.title('Training and validation loss')
plt.legend()
plt.show()

acc_values = history.history['accuracy']
val_acc_values = history.history['val_accuracy']

plt.plot(epochs, acc_values, 'bo', label='Training Acc')
plt.plot(epochs, val_acc_values, 'b', label='Validation Acc')
plt.title("Training and Validation Accuracy")
plt.xlabel("Epochs")
plt.ylabel("Accuracy")
plt.legend()
plt.show()

test_loss, test_acc = model.evaluate(test_values, test_labels)
print('test_acc: ', test_acc)

# Plot predictions against actual values
predictions = model.predict(test_values)
pred_labels = [ 1 if i >= 0.5 else 0 for i in predictions[:,0] ]


plt.figure(figsize=(8,8))
plt.title("Comparison of predictions to actual values")
plt.scatter(test_values[:,[0]], test_values[:,[1]], c=test_labels[:,[1]], cmap=colors.ListedColormap(['yellow','blue']))
plt.scatter(test_values[:,[0]], test_values[:,[1]], c=pred_labels, cmap=colors.ListedColormap(['blue','yellow']), alpha=0.7)
plt.legend()
plt.show()





