# -*- coding: utf-8 -*-
"""
Criado em Mon Apr  5 21:13:38 2021

@author: Jasmine Moreira

2022
pip install torch==1.7.1+cu110 torchvision==0.8.2+cu110 torchaudio==0.7.2 -f https://download.pytorch.org/whl/torch_stable.html
"""
import torch
import time 

t = 1000
dim = []
cpu = []
gpu = []

for n in (1,5,10,20,40,80,160,320,640,1280,2560,5120,10240,20480):

    start = time.time()
    b = torch.ones(n,n)
    for _ in range(t):
        b += b  
    c = time.time()-start
    
    start = time.time()
    b = torch.ones(n,n).cuda()
    for _ in range(t):
        b += b  
    g = time.time()-start
    
    dim.append(n)
    cpu.append(c)
    gpu.append(g)
    
    
import matplotlib.pyplot as plt
plt.scatter(dim,cpu, c='b', marker='x', label='cpu')
plt.scatter(dim,gpu, c='r', marker='s', label='gpu')
plt.legend(loc='upper left')
plt.xscale('log')
plt.yscale('log')
plt.show()