import os
import sys
import pathlib
import multiprocessing as mp
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


PYTHON_VERSION = '.'.join(sys.version.split('.')[:2])

class build_g2opy(build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)

    def build_cmake(self, ext):
        cwd = pathlib.Path(ext.sources[0]).absolute()

        build_temp = pathlib.Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = pathlib.Path(self.get_ext_fullpath(ext.sources[0]))
        extdir.mkdir(parents=True, exist_ok=True)

        config = 'Debug' if self.debug else 'Release'
        cmake_args = [
            f'-DCMAKE_BUILD_TYPE={config}',
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={str(extdir.parent.absolute())}',
            f'-Dg2o_LIBRARY_OUTPUT_DIRECTORY={str(extdir.parent.absolute())}',
            '-DG2O_USE_OPENGL=0',
            '-DG2O_BUILD_APPS=0',
            '-DG2O_BUILD_EXAMPLES=0',
            '-DG2O_BUILD_PYTHON=1',
            f'-DPYBIND11_PYTHON_VERSION={PYTHON_VERSION}'
        ]

        build_args = [
            '--config', config,
            '--', f'-j{mp.cpu_count()}'
        ]

        os.chdir(str(build_temp))
        self.spawn(['cmake', str(cwd)] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.'] + build_args)
        os.chdir(str(cwd))

setup(
    name='g2opy',
    version='0.1.0',
    description='Python binding of C++ graph optimization framework g2o.',
    url='https://github.com/c7huang/g2o',
    license='BSD',
    packages=[],
    ext_modules=[Extension('g2opy', sources=['.'])],
    cmdclass=dict(build_ext=build_g2opy),
    keywords='g2o, SLAM, BA, ICP, optimization, python, binding',
    long_description="""This is a Python binding for c++ library g2o 
        (https://github.com/RainerKuemmerle/g2o).

        g2o is an open-source C++ framework for optimizing graph-based nonlinear 
        error functions. g2o has been designed to be easily extensible to a wide 
        range of problems and a new problem typically can be specified in a few 
        lines of code. The current implementation provides solutions to several 
        variants of SLAM and BA."""
)
