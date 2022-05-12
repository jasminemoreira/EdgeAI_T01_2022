# -*- coding: utf-8 -*-
"""
Created on Sun May  1 21:58:56 2022

@author: Jasmine Moreira
"""

import numpy as np
import matplotlib.pyplot as plt
import math
import tensorflow as tf
from tensorflow.keras import layers
#Add Path to Spyder ---> Tools->PYTHONPATH manager
from hex_to_c_array import hex_to_c_array


# Settings
nsamples = 2000     # Number of samples to use as a dataset
val_ratio = 0.2     # Percentage of samples that should be held for validation set
test_ratio = 0.2    # Percentage of samples that should be held for test set

tflite_name = r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\model_camparison\sine_model.tflite"
model_h_name = r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\ESP-32\my_first_tinyml\sine_model.h"

# Generate some random samples
np.random.seed(1234)
x_values = np.random.uniform(low=0, high=(2 * math.pi), size=nsamples)
plt.figure(figsize=(8,8))
plt.title("Uniform Random Set to Generate Sin")
plt.plot(x_values)

# Create a noisy sinewave with these values
y_values = np.sin(x_values) + (0.1 * np.random.randn(x_values.shape[0]))

#Plot before normalization
plt.figure(figsize=(8,8))
plt.title("Before normalization Set")
plt.plot(x_values, y_values, '.')
plt.show()


# normalize
x_values = x_values - np.mean(x_values)
x_factor = np.max([-1*np.min(x_values),np.max(x_values)])
y_factor = np.max([-1*np.min(y_values),np.max(y_values)])
x_values = (x_values)/x_factor
y_values = (y_values)/y_factor

#x_mean = np.mean(x_values)
#x_stdd = np.std(x_values)
#x_values = (x_values-x_mean)/x_stdd

plt.figure(figsize=(8,8))
plt.title("Normalized Set")
plt.plot(x_values, y_values, '.')
plt.show()


# Split the dataset into training, validation, and test sets
val_split = int(val_ratio * nsamples)
test_split = int(val_split + (test_ratio * nsamples))
x_val, x_test, x_train = np.split(x_values, [val_split, test_split])
y_val, y_test, y_train = np.split(y_values, [val_split, test_split])

# Check that our splits add up correctly
assert(x_train.size + x_val.size + x_test.size) == nsamples

# Plot the data in each partition in different colors:
plt.figure(figsize=(8,8))
plt.title("Splited Set")
plt.plot(x_train, y_train, 'b.', label="Train")
plt.plot(x_test, y_test, 'r.', label="Test")
plt.plot(x_val, y_val, 'y.', label="Validate")
plt.legend()
plt.show()

# Create a model
model = tf.keras.Sequential()
model.add(layers.Dense(6, activation='relu', input_shape=(1,)))
model.add(layers.Dense(4, activation='relu'))
model.add(layers.Dense(1))

# View model
model.summary()


# Add optimizer, loss function, and metrics to model and compile it
model.compile(optimizer='adam', loss='mae', metrics=['mae'])

# Train model
history = model.fit(x_train,
                    y_train,
                    epochs=300,
                    batch_size=20,
                    validation_data=(x_val, y_val))

# Plot the training history
loss = history.history['loss']
val_loss = history.history['val_loss']

#loss = loss[180:]
#val_loss = val_loss[180:]

epochs = range(1, len(loss) + 1)
plt.figure(figsize=(8,8))
plt.plot(epochs, loss, 'bo', label='Training loss')
plt.plot(epochs, val_loss, 'b', label='Validation loss')
plt.title('Training and validation loss')
plt.legend()
plt.show()


# Plot predictions against actual values
predictions = model.predict(x_test)

#Plot denormalized
plt.figure(figsize=(8,8))
plt.title("Denormalized Predictions vs Actual Values")
plt.plot(x_test * x_factor, y_test * y_factor, 'b.', label='Actual')
plt.plot(x_test * x_factor, predictions * y_factor, 'r.', label='Prediction')
plt.legend()
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

open(tflite_name, 'wb').write(tflite_model)

# Write TFLite model to a C source (or header) file
with open(model_h_name, 'w') as file:
  file.write(hex_to_c_array(tflite_model, "sine_model", x_factor, y_factor))

