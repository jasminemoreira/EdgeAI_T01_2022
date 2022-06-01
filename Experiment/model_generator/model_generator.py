# -*- coding: utf-8 -*-
"""
Created on Fri May 20 18:33:54 2022

@author: Jasmine Moreira
"""

import csv
import numpy as np
import matplotlib.pyplot as plt
import tensorflow as tf
from tensorflow.keras import models, layers
from tensorflow.keras.utils import to_categorical
#Add Path to Spyder ---> Tools->PYTHONPATH manager
from hex_to_c_array import hex_to_c_array

window_size = 60
data = []

with open(r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\Experiment\model_generator\raw.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=' ', quoting=csv.QUOTE_NONNUMERIC)
    for row in csv_reader:
        data.append(row)         
       

train_images = []
train_labels = []  
test_images = []
test_labels = []
for x in range(0,len(data)-window_size):
    img = []
    label = ''
    for y in range(x,x+window_size):
        img.append(data[y])
  
    deltax = max(list(zip(*img))[0]) - min(list(zip(*img))[0])
    deltay = max(list(zip(*img))[1]) - min(list(zip(*img))[1])
    deltaz = max(list(zip(*img))[2]) - min(list(zip(*img))[2])
    
    label = 0 if (deltax > deltay and deltax > deltaz and deltax > 2) else \
            1 if (deltay > deltaz and deltay > deltax and deltay > 2) else \
            2 if (deltaz > deltay and deltaz > deltax and deltaz > 2) else 3
            
    if(np.random.random()<0.8):
        train_images.append(img)
        train_labels.append(label)
    else:
        test_images.append(img)
        test_labels.append(label)            

data = None

train_labels = to_categorical(train_labels)
test_labels = to_categorical(test_labels)

train_images = np.array(train_images).astype('float32').reshape((len(train_images),window_size,3,1))
test_images = np.array(test_images).astype('float32').reshape((len(test_images),window_size,3,1))


# criar modelo
model = models.Sequential()
model.add(layers.Conv2D(4, (3,1),activation='relu', input_shape=(window_size,3,1)))
model.add(layers.MaxPooling2D((2,2)))
model.add(layers.Flatten())
model.add(layers.Dense(4, activation = 'softmax'))
model.compile(optimizer='adam',loss='categorical_crossentropy', metrics=['accuracy'])
model.summary()

history = model.fit(train_images, 
                      train_labels, 
                      epochs=20, 
                      batch_size=10, 
                      validation_data=(test_images,test_labels))


test_loss, test_acc = model.evaluate(test_images, test_labels)

history_dict = history.history
loss_values = history_dict['loss']
val_loss_values = history_dict['val_loss']

epochs = range(1, len( history_dict['loss']) + 1)

plt.plot(epochs, loss_values, 'bo', label='Training Loss')
plt.plot(epochs, val_loss_values, 'b', label='Validation Loss')
plt.title("Training and Validation Loss")
plt.xlabel("Epochs")
plt.ylabel("Loss")
plt.legend()
plt.show()

acc_values = history_dict['accuracy']
val_acc_values = history_dict['val_accuracy']

plt.plot(epochs, acc_values, 'bo', label='Training Acc')
plt.plot(epochs, val_acc_values, 'b', label='Validation Acc')
plt.title("Training and Validation Accuracy")
plt.xlabel("Epochs")
plt.ylabel("Accuracy")
plt.legend()
plt.show()

plt.figure(figsize=(8,8))
plt.imshow(train_images[101],cmap='binary')
plt.show()




def representative_dataset_generator():
    global x_test
    for value in x_test: # Each scalar value must be inside of a 2D array that
        yield [np.array( value, dtype = np.float32, ndmin = 2)]
        
# Convert Keras model to a tflite model
converter = tf.lite.TFLiteConverter.from_keras_model(model)
       
#Quantization - Must check if it is worth!
converter.representative_dataset = representative_dataset_generator
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.optimizations = [tf.lite.Optimize.EXPERIMENTAL_SPARSITY]

tflite_model = converter.convert()

tflite_name = r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\Experiment\model_generator\xyz_model.tflite"
model_h_name = r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\Experiment\xyz_detector\xyz_model.h"

open(tflite_name, 'wb').write(tflite_model)

# Write TFLite model to a C source (or header) file
with open(model_h_name, 'w') as file:
  file.write(hex_to_c_array(tflite_model, "xyz_model", 1, 1))



