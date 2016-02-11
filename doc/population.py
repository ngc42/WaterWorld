#!/usr/bin/python3

kMaxPop = 16.0
kLimit = 4.0
kLimitSqr = 16.0

population = 100


def Population():
  global population
  g = 0.4;
  p1 = population/1000000;
  if p1>1:
    x = p1 / (kMaxPop*kLimit)
    x = (-(x*x)) + (1 / kLimitSqr)
    p1 = population*(x*g)
  else:
    x = p1 / 0.2+5
    x = 10 / (x*x+20)
    p1 = population*(x*g)
  
  if p1 < 1 :
    p1 += 1.5
  
  population = population + p1
  
  
for round in range(0, 1000):
  print(round, population)
  Population()
  

