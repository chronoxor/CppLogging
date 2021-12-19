#!/usr/bin/env python3

import distutils.core

if __name__ == "__main__":
    distutils.core.setup(
        name="make-hashlog",
        version="1.0.0.0",
        author="Ivan Shynkarenka",
        author_email="chronoxor@gmail.com",
        url="https://github.com/chronoxor/CppLogging/tools/make-hashlog",
        download_url = "https://github.com/chronoxor/CppLogging/archive/refs/tags/1.0.0.0.tar.gz",
        description="Gil (git links) tool",
        long_description="Gil (git links) tool allows to describe and manage complex git repositories dependencies with cycles and cross references",
        license="MIT License",
        scripts=["gil.py", "gil.bat", "gil"],
        platforms="Linux, MacOS, Windows",
        classifiers=[
            "Operating System :: POSIX",
            "Operating System :: MacOS :: MacOS X",
            "Operating System :: Microsoft :: Windows",
            "Programming Language :: Python :: 3"
        ]
    )
