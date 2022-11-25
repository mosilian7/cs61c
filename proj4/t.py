
import numc as nc
import time
import numpy as np
import random

rows = 2000
cols = 2000
rows_ = 299
cols_ = 135

check_row = 177
check_col = 134

def random_list_2d(rows, cols):
	l = [0] * rows
	for i in range(rows):
		row = [0] * cols
		for j in range(cols):
			row[j] = 2 * random.random() - 1
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

a1 = np.mat(l1)
a2 = np.mat(l2)

start = time.time()
result_m = m1 ** 4
end = time.time()
m_time = end - start
print(f"numc:  {m_time} s")

start = time.time()
result_a = a1 ** 4
end = time.time()
a_time = end - start
print(f"numpy: {a_time} s")

print("---------speed-up summary---------")
print(f"numc / numpy: {m_time/a_time}x")


print("---------correctness check--------")
print("numc  result: ", result_m[check_row][check_col])
print("numpy result: ", result_a[check_row, check_col])
