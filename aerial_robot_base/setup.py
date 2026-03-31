from setuptools import find_packages, setup


setup(
    name='aerial_robot_base',
    version='1.3.6',
    packages=find_packages(where='src'),
    package_dir={'': 'src'},
)
