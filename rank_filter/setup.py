from setuptools import setup
from setuptools.extension import Extension
from Cython.Distutils import build_ext
import numpy as np

ext_modules = [Extension("rank_filter_1d", ["rank_filter_1d.pyx"],
                         include_dirs=[np.get_include()],
                         extra_compile_args=["-std=c++11", "-O1"],
                         language="c++")]

setup(
    name='Rank Filter 1D Cython',
    cmdclass={'build_ext': build_ext},
    ext_modules=ext_modules
)