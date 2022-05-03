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


# Settings
nsamples = 1000     # Number of samples to use as a dataset
val_ratio = 0.2     # Percentage of samples that should be held for validation set
test_ratio = 0.2    # Percentage of samples that should be held for test set
#tflite_model_name = 'sine_model'  # Will be given .tflite suffix
#c_model_name = 'sine_model'       # Will be given .h suffix


# Generate some random samples
np.random.seed(1234)
x_values = np.random.uniform(low=0, high=(2 * math.pi), size=nsamples)
plt.figure(figsize=(8,8))
plt.plot(x_values)


# Create a noisy sinewave with these values
y_values = np.sin(x_values) + (0.1 * np.random.randn(x_values.shape[0]))
plt.figure(figsize=(8,8))
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
plt.plot(x_train, y_train, 'b.', label="Train")
plt.plot(x_test, y_test, 'r.', label="Test")
plt.plot(x_val, y_val, 'y.', label="Validate")
plt.legend()
plt.show()


# Create a model
model = tf.keras.Sequential()
model.add(layers.Dense(50, activation='relu', input_shape=(1,)))
model.add(layers.Dense(10, activation='relu'))
model.add(layers.Dense(1))

# View model
model.summary()


# Add optimizer, loss function, and metrics to model and compile it
model.compile(optimizer='adam', loss='mse', metrics=['mae'])

# Train model
history = model.fit(x_train,
                    y_train,
                    epochs=400,
                    batch_size=30,
                    validation_data=(x_val, y_val))

# Plot the training history
loss = history.history['loss']
val_loss = history.history['val_loss']


epochs = range(1, len(loss) + 1)
plt.figure(figsize=(8,8))
plt.plot(epochs, loss, 'bo', label='Training loss')
plt.plot(epochs, val_loss, 'b', label='Validation loss')
plt.title('Training and validation loss')
plt.legend()
plt.show()


# Plot predictions against actual values
predictions = model.predict(x_test)

plt.figure(figsize=(8,8))
plt.title("Comparison of predictions to actual values")
plt.plot(x_test, y_test, 'b.', label='Actual')
plt.plot(x_test, predictions, 'r.', label='Prediction')
plt.legend()
plt.show()