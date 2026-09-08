from setuptools import setup, find_packages

setup(
    name='libhalplatformclient-otn',
    version='1.0.0',
    description='Hal platform api thrift client for SONiC platform',
    long_description='Hal platform API Thrift client for SONiC platform',
    license='Apache 2.0',
    author='Molex',
    author_email='lu.mao@molex.com',
    url='https://github.com/Azure/sonic-buildimage',
    maintainer='Molex',
    maintainer_email='lu.mao@molex.com',
    packages=find_packages(),
    install_requires=[
        'thrift>=0.14.0',  # adjust as per your actual dependency version
    ],
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Environment :: Plugins',
        'Intended Audience :: Developers',
        'Intended Audience :: Information Technology',
        'Intended Audience :: System Administrators',
        'License :: OSI Approved :: Apache Software License',
        'Natural Language :: English',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python :: 3.8',
        'Topic :: Utilities',
    ],
    keywords='sonic SONiC platform PLATFORM hal HAL',
)

