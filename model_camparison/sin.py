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

base_name = r"C:\Users\jasmi\INDT\Cursos\EdgeAI_T01_2022\model_camparison\sine_model"


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

#x_factor = 1
#y_factor = 1

# normalize
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
model.add(layers.Dense(16, activation='relu', input_shape=(1,)))
model.add(layers.Dense(16, activation='relu'))
model.add(layers.Dense(1))

# View model
model.summary()


# Add optimizer, loss function, and metrics to model and compile it
model.compile(optimizer='adam', loss='mae', metrics=['mae'])

# Train model
history = model.fit(x_train,
                    y_train,
                    epochs=200,
                    batch_size=20,
                    validation_data=(x_val, y_val))

# Plot the training history
loss = history.history['loss']
val_loss = history.history['val_loss']

#loss = loss[100:]
#val_loss = val_loss[100:]

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


# Convert Keras model to a tflite model
converter = tf.lite.TFLiteConverter.from_keras_model(model)
#converter.optimizations = [tf.lite.Optimize.OPTIMIZE_FOR_SIZE]
#converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

open(base_name + '.tflite', 'wb').write(tflite_model)


# Function: Convert some hex value into an array for C programming
def hex_to_c_array(hex_data, var_name):

  c_str = ''

  # Create header guard
  #c_str += '#ifndef ' + var_name.upper() + '_H\n'
  #c_str += '#define ' + var_name.upper() + '_H\n\n'

  # Add denormalization factors
  c_str += 'float x_factor = ' + str(x_factor) + ';\n'
  c_str += 'float y_factor = ' + str(y_factor) + ';\n'

  # Add array length at top of file
  #c_str += '\nunsigned int ' + var_name + '_len = ' + str(len(hex_data)) + ';\n'

  # Declare C variable
  c_str += 'unsigned char ' + var_name + '[] = {'
  hex_array = []
  for i, val in enumerate(hex_data) :

    # Construct string from hex
    hex_str = format(val, '#04x')

    # Add formatting so each line stays within 80 characters
    if (i + 1) < len(hex_data):
      hex_str += ','
    if (i + 1) % 12 == 0:
      hex_str += '\n '
    hex_array.append(hex_str)

  # Add closing brace
  c_str += '\n ' + format(' '.join(hex_array)) + '\n};\n\n'

  # Close out header guard
  #c_str += '#endif //' + var_name.upper() + '_H'

  return c_str

# Write TFLite model to a C source (or header) file
with open(base_name + '.h', 'w') as file:
  file.write(hex_to_c_array(tflite_model, "sine_model"))

