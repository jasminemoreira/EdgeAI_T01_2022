# -*- coding: utf-8 -*-
"""
Created on Thu May 12 08:34:50 2022

@author: jasmi
"""

# Function: Convert some hex value into an array for C programming
def hex_to_c_array(hex_data, var_name, x_factor, y_factor):

  c_str = ''

  # Create header guard
  #c_str += '#ifndef ' + var_name.upper() + '_H\n'
  #c_str += '#define ' + var_name.upper() + '_H\n\n'

  # Add denormalization factors
  c_str += 'float x_factor = ' + str(x_factor) + ';\n'
  c_str += 'float y_factor = ' + str(y_factor) + ';\n'

  # Add array length at top of file
  #c_str += '\nunsigned int ' + var_name + '_len = ' + str(len(hex_data)) + ';\n'

  # Declare C variable and make it aligned in memory
  c_str += 'unsigned char ' + var_name + '[] __attribute__((aligned(4)))= {'
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