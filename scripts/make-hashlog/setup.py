#!/usr/bin/env python3

import distutils.core

if __name__ == "__main__":
    distutils.core.setup(
        name="make-hashlog",
        version="1.1.0.0",
        author="Ivan Shynkarenka",
        author_email="chronoxor@gmail.com",
        url="https://github.com/chronoxor/CppLogging/tools/make-hashlog",
        download_url = "https://github.com/chronoxor/CppLogging/archive/refs/tags/1.0.0.0.tar.gz",
        description="Make hashlog tool",
        long_description="Make hashlog tool is used to parse C++ source files for logging messages and create corresponding .hashlog binary file",
        license="MIT License",
        scripts=["make-hashlog.py", "make-hashlog.bat", "make-hashlog"],
        platforms="Linux, MacOS, Windows",
        classifiers=[
            "Operating System :: POSIX",
            "Operating System :: MacOS :: MacOS X",
            "Operating System :: Microsoft :: Windows",
            "Programming Language :: Python :: 3"
        ]
    )
