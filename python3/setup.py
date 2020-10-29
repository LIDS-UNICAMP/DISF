from distutils.core import setup, Extension
from glob import glob
import numpy


disf_module = Extension("disf",
                sources = ["python3/DISF_py3.c"],
                include_dirs = ["./include", numpy.get_include()],
                library_dirs = ["./lib"],
                libraries = ["disf"],
                extra_compile_args = ["-O3", "-fopenmp"],
                extra_link_args=['-lgomp']
              );    

setup(name = "DISF", 
      version = "1.0",
      author = "Felipe Belem",
      description = "Dynamic and Iterative Spanning Forest for superpixel segmentation",
      ext_modules = [disf_module]);