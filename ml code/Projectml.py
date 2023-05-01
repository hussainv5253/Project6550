#!/usr/bin/python3
import numpy as np
import tensorflow as tf

from random import random

from tensorflow.keras import layers
from tensorflow.keras import models

N = 100000

training_input = np.zeros(shape=(N,2))
training_output = np.zeros(shape=(N,1))

for i in range(N):
    temp = random()*50
    humidity = random()*100
    training_input[i] = [temp, humidity]

    if ((humidity < (2 * temp) + 10)
        training_output[i] = 1
    if (temp > 35)
        training_output[i] = 1

model = models.Sequential();
model.add(layers.Dense(16, input_shape=(2,), activation='relu'))
model.add(layers.Dense(1))

model.summary()

model.compile(loss='mean_squared_error', optimizer='adam', metrics=['accuracy'])

model.fit(training_input, training_output, epochs=3)

O = 10

testing_input = np.zeros(shape=(O,2))

for i in range(O):
    temp = random()*50
    humidity = random()*100
    testing_input[i] = [temp, humidity]

testing_output = (abs(model.predict(testing_input)).round())

for i in range(O):
    print("Temp: " + f"{testing_input[i][0]:.2f}".rjust(6) + ", Humidity: " + f"{testing_input[i][1]:.2f}".rjust(5) + ", Humidifier: " + str(testing_output[i][0]))

line = "\n#define MAX_WEIGHT " + str(model.layers[0].weights[0].shape[1]) + "\n"

line += "\ndouble w11[MAX_WEIGHT] = { "

for i in range(model.layers[0].weights[0].shape[1]):
    if i > 0:
        line += ","
    line += str(model.layers[0].weights[0][0][i].numpy())

line += " };\n"

line += "\ndouble w12[MAX_WEIGHT] = { "

for i in range(model.layers[0].weights[0].shape[1]):
    if i > 0:
        line += ","
    line += str(model.layers[0].weights[0][1][i].numpy())

line += " };\n"

line += "\ndouble b1[MAX_WEIGHT] = { "

for i in range(model.layers[0].weights[1].shape[0]):
    if i > 0:
        line += ","
    line += str(model.layers[0].weights[1][i].numpy())

line += " };\n"

line += "\ndouble w2[MAX_WEIGHT] = { "

for i in range(model.layers[1].weights[0].shape[0]):
    if i > 0:
        line += ","
    line += str(model.layers[1].weights[0][i][0].numpy())

line += " };\n"

line += "\ndouble b2 = " + str(model.layers[1].weights[1][0].numpy()) + ";"

print(line)
