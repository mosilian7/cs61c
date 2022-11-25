from distutils.core import setup, Extension
import sysconfig

def main():
    CFLAGS = ['-g', '-Wall', '-std=c11', '-fopenmp', '-mavx', '-mfma', '-pthread', '-O3']
    LDFLAGS = ['-fopenmp']
    # Use the setup function we imported and set up the modules.
    # You may find this reference helpful: https://docs.python.org/3.6/extending/building.html
    # TODO: YOUR CODE HERE
    mod = Extension('numc', sources=['numc.c', 'matrix.c'],
    #include_dirs=['/home/mosilian/.conda/envs/61c/include/python3.6m'],
    #library_dirs=['/usr/lib/x86_64-linux-gnu'],
    #libraries=['python3.6m'],
    extra_compile_args=CFLAGS, extra_link_args=LDFLAGS)
    setup(name='numc', version='1.0', description='first version of numc', ext_modules=[mod])

if __name__ == "__main__":
    main()