from distutils.core import setup, Extension

setup(
    name = "python-simhash",
    version = "0.1",
    description = "An efficient implementation of Charikar's simhash for Python",
    author="Shane Evans",
    packages = ["simhash"],
    ext_modules = [
        Extension("_simhash",
        sources=["simhash/_simhash.c"],
        include_dirs=['simhash'])
    ]
)
