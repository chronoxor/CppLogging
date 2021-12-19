#!/usr/bin/env python3

import distutils.core

if __name__ == "__main__":
    distutils.core.setup(
        name="hashlog-map",
        version="1.2.0.0",
        author="Ivan Shynkarenka",
        author_email="chronoxor@gmail.com",
        url="https://github.com/chronoxor/CppLogging/scripts/hashlog-map",
        download_url = "https://github.com/chronoxor/CppLogging/archive/refs/tags/1.2.0.0.tar.gz",
        description="Hashlog map generate tool",
        long_description="Hashlog map generate tool is used to parse C++ source files for logging format messages and create the corresponding .hashlog binary file",
        license="MIT License",
        scripts=["hashlog-map.py", "hashlog-map.bat", "hashlog-map"],
        platforms="Linux, MacOS, Windows",
        classifiers=[
            "Operating System :: POSIX",
            "Operating System :: MacOS :: MacOS X",
            "Operating System :: Microsoft :: Windows",
            "Programming Language :: Python :: 3"
        ]
    )
