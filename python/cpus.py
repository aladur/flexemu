#!/usr/bin/env python

import multiprocessing

def main():
    cpu_count: int = multiprocessing.cpu_count()
    print(f"{cpu_count}")

if __name__ == "__main__":
    main()

