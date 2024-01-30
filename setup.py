from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy as np

# Specify the Cython extension module
ext_modules = [
    Extension(
        "heap_cy",  # Replace with the name of your Cython module
        sources=["heap_cy.pyx"],  # Replace with the name of your Cython source file
        include_dirs=[np.get_include()],  # Include NumPy headers
        language="c",
    ),
Extension(
        "rolling_mad_cy",  # Replace with the name of your Cython module
        sources=["rolling_mad_cy.pyx"],  # Replace with the name of your Cython source file
        include_dirs=[np.get_include()],  # Include NumPy headers
        language="c",
    ),
Extension(
        "mad_cy",  # Replace with the name of your Cython module
        sources=["mad_cy.pyx"],  # Replace with the name of your Cython source file
        include_dirs=[np.get_include()],  # Include NumPy headers
        language="c",
    )
]

# Setuptools setup function
setup(
    name="your_package_name",  # Replace with your package name
    ext_modules=cythonize(ext_modules),
    include_dirs=[np.get_include()],  # Include NumPy headers
    install_requires=[
        "numpy",  "scipy", "pandas", "matplotlib" # Specify NumPy as a dependency
        # Add other dependencies if needed
    ],
    # Other setup parameters...
)
