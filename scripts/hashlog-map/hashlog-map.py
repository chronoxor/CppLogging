#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Hashlog map generate tool is used to parse C++ source files for logging format messages
and create the corresponding .hashlog binary file
"""

import codecs
import glob
import os
import re
import struct
import sys

__author__ = "Ivan Shynkarenka"
__email__ = "chronoxor@gmail.com"
__license__ = "MIT License"
__url__ = "https://github.com/chronoxor/CppLogging/scripts/hashlog-map"
__version__ = "1.4.0.0"


class HashLogContext(object):
    def __init__(self, path):
        self.hash = dict()
        self.path = os.path.abspath(path)

    def hash_fnv1a_32(self, message):
        FNV_PRIME = 16777619
        OFFSET_BASIS = 2166136261
        hash = OFFSET_BASIS
        for char in message:
            hash ^= ord(char)
            hash *= FNV_PRIME
            hash %= 2**32
        return hash

    def discover(self, path):
        path = os.path.abspath(path)
        path = os.path.join(path, "**")

        # Recursive discover source files of required extensions
        sources = []
        for extension in ["*.h", "*.inl", "*.cpp"]:
            sources.extend(glob.glob(os.path.join(path, extension), recursive=True))

        # Find logging messages using pattern match
        pattern = re.compile('((Debug)|(Info)|(Warn)|(Error)|(Fatal))\\("(.*)", ')
        for source in sources:
            for i, line in enumerate(open(source)):
                for match in re.finditer(pattern, line):
                    message = codecs.decode(match.group(7), "unicode_escape")
                    hash = self.hash_fnv1a_32(message)
                    if hash not in self.hash:
                        print('Discovered logging message: "%s" with hash = 0x%08X' % (message, hash))
                        self.hash[hash] = message
                    elif message != self.hash[hash]:
                        print("Collision detected!", file=sys.stderr)
                        print('Previous logging message: "%s" with hash = 0x%08X' % (self.hash[hash], hash), file=sys.stderr)
                        print('Conflict logging message: "%s" with hash = 0x%08X' % (message, hash), file=sys.stderr)
                        raise Exception("Collision detected!")

    def generate(self, path):
        path = os.path.abspath(path)
        path = os.path.join(path, ".hashlog")

        # Write the current hash map into the binary file
        print("Generating .hashlog file", end="...")
        with open(path, "wb") as hashlog:
            hashlog.write(struct.pack("<I", len(self.hash)))
            for hash, message in self.hash.items():
                data = bytearray(message.encode("utf-8"))
                hashlog.write(struct.pack("<I", hash))
                hashlog.write(struct.pack("<I", len(data)))
                hashlog.write(data)
        print("Done!")

    def view(self, path):
        path = os.path.abspath(path)
        path = os.path.join(path, ".hashlog")

        # Read the binary file with hash map
        with open(path, "rb") as hashlog:
            size = struct.unpack("<I", hashlog.read(4))[0]
            for _ in range(size):
                hash = struct.unpack("<I", hashlog.read(4))[0]
                length = struct.unpack("<I", hashlog.read(4))[0]
                data = hashlog.read(length)
                message = data.decode("utf-8")
                print('0x%08X: %s' % (hash, message))


def show_help():
    print("usage: hashlog-map command")
    print("Supported commands:")
    print("\thelp - show this help")
    print("\tversion - show version")
    print("\tgenerate - generate .hashlog")
    print("\tview - view .hashlog")
    sys.exit(1)


def show_version():
    print(__version__)
    sys.exit(1)


def main():
    # Show help message
    if len(sys.argv) <= 1:
        show_help()

    if sys.argv[1] == "help":
        show_help()
    elif sys.argv[1] == "version":
        show_version()

    # Get the current working directory
    path = os.getcwd()

    # Create hash log context
    context = HashLogContext(path)

    if sys.argv[1] == "generate":
        context.discover(path)
        context.generate(path)
    elif sys.argv[1] == "view":
        context.view(path)
    else:
        print("Unknown command: %s" % sys.argv[1])
        return -1
    return 0


if __name__ == "__main__":
    sys.exit(main())
