#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Sep 25 18:31:40 2018

@author: jasmine

1) Preparar dados
2) Criar o modelo (input, output size, forward pass)
3) Criar a função de erro (loss) e o otimizador 
4) Criar o loop de treinamento
   - forward pass: calcular a predição e o erro
   - backward pass: calcular os gradientes
   - update weights: ajuste dos pesos do modelo
"""

input_vec = range(1,11)
categories = (0,0,0,0,0,1,1,1,1,1)

weight = 0
bias = 0
step = 0.001
error_log = []

def out(input):
    pot = (input*weight+bias) 
    return min(max(0,pot),1) #sigmoid

def error():
    error = 0
    for input,category in zip(input_vec,categories):
        error = error + (out(input)-category)**2
    return error/len(input_vec)

#training
e = error()
print("erro inicial:"+str(e))
for i in range(1,10000):
    e = error()
    weight = weight+step
    if error() >= e: weight = weight-2*step
    e = error()
    bias = bias+step
    if error() >= e: bias = bias-2*step
    if not i%100: 
        error_log.append(e)
        print("error:"+str(e))

#get class
def getClass(val):
    return 1 if out(val)>=0.5 else 0

test_vec = (1,10,2,9,3,8,4,7,5,6)
for input in test_vec:
    print("class: "+str(out(input))+"   value: "+str(input))


#return min(max(0,pot),1)  #satured relu
#return (0,1)[pot > 0] #step
#return max(0,pot)
#return 1/(1+ 2.7182818284**(-pot)) #sigmoid

import matplotlib.pyplot as plt
plt.plot(error_log, 'bo', label='Training Error')
plt.title("Error during training")
plt.ylabel("Error")
plt.legend()
plt.show()

