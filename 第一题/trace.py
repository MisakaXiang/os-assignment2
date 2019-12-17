import matplotlib.pyplot as plt
import numpy as np
import time
addresses = []
add_time = []


def fun1(list1):
    sum_num = []
    for i in range(len(list1)):
        sum_num.append(list1[i])

        addresses.append(id(list1[i]))
        # add_time.append(time.time())
        # time.sleep(0.001)
        addresses.append(id(sum_num[i]))
        # add_time.append(time.time())
        # time.sleep(0.0001)
    return sum_num


def fun2(list2):
    sum_num = []
    # sum_num = np.zeros([6400])
    for i in range(len(list2)):
        for j in range(len(list2[0])):
                sum_num.append(list2[i][j])
                addresses.append(id(list2[i][j]))
                # add_time.append(time.time())
                # time.sleep(0.001)
                addresses.append(id(sum_num[i]))
                # add_time.append(time.time())
                # time.sleep(0.0001)
    return sum_num

def fun3(list3):
    sum_num = []
    for i in range(len(list3)):
        for j in range(len(list3[0])):
            for k in range(len(list3[0][0])):
                sum_num.append(list3[i][j][k])
                addresses.append(id(list3[i][j][k]))
                # add_time.append(time.time())
                # time.sleep(0.001)
                addresses.append(id(sum_num[i]))
                # add_time.append(time.time())
                # time.sleep(0.0001)
    return sum_num


list1 = np.zeros([1400])
for i in range(1400):
    list1[i] = 3

list2 = np.zeros([40, 40])
for i in range(40):
    for j in range(40):
        list2[i][j] = 1

list3 = np.zeros([10, 20, 10])
for i in range(10):
    for j in range(20):
        for k in range(10):
            list3[i][j] = 1


fun1(list1)
fun2(list2)
fun3(list3)
for i in range(10000):
    add_time.append(i)
f = open(r'addresses-locality.txt', 'w')
for i in range(len(addresses)):
    print(addresses[i], file=f)
f.close()
plt.scatter(add_time, addresses)
plt.show()