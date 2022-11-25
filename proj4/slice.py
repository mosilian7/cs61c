import numc as nc 

a = nc.Matrix(4,4)
for i in range(4):
	for j in range(4):
		a[i, j] = 4 * i + j;

print(a)

b = a[1:4, 1:4]
c = b[0:2]

print(b)
print(c)

print('-----------------')

c[0:1,0] = -1

print(a)
print(b)
print(c)

a = 0

print('-----------------')

c[0:2, 0:2] = [[-6,-7],[-8,-9]]

print(b)
print(c)

b = 0
print('-----------------')
print(c)
c=0