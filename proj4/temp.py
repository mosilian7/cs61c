
import numc as nc
import time
import numpy as np
import random

rows = 1024
cols = 1024
rows_ = 1024
cols_ = 1024

check_row = 277
check_col = 61

def random_list_2d(rows, cols):
	l = [0] * rows
	for i in range(rows):
		row = [0] * cols
		for j in range(cols):
			row[j] = random.random()
		l[i] = row
	return l

def zero_list_2d(rows,cols):
	l = []
	for i in range(rows):
		row = [0] * cols
		l.append(row)
	return l

l1 = random_list_2d(rows, cols)
l2 = random_list_2d(rows_, cols_)

m1 = nc.Matrix(l1)
m2 = nc.Matrix(l2)

a1 = np.array(l1)
a2 = np.array(l2)

result_l = zero_list_2d(rows, cols_)


start = time.time()
for i in range(rows):
	for k in range(cols):
		for j in range(cols_):
			result_l[i][j] += l1[i][k] * l2[k][j]
end = time.time()
l_time = end - start
print(f"list:  {l_time} s")


start = time.time()
result_m = m1 * m2
end = time.time()
m_time = end - start
print(f"numc:  {m_time} s")

start = time.time()
result_a = np.dot(a1,a2)
end = time.time()
a_time = end - start
print(f"numpy: {a_time} s")

print("---------speed-up summary---------")
print(f"numc  speed-up: {l_time/m_time}x")
print(f"numpy speed-up: {l_time/a_time}x")
print(f"numc / numpy: {m_time/a_time}x")


print("---------correctness check--------")
print("list  result: ", result_l[check_row][check_col])
print("numc  result: ", result_m[check_row][check_col])
print("numpy result: ", result_a[check_row][check_col])
