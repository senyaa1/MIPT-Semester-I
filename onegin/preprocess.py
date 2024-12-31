#!/usr/bin/env python3

with open("onegin.txt", "r") as input_file:
    with open("onegin_proccessed.txt", "w") as output_file:
        A = [x.rstrip() for x in input_file.readlines()]

        for line in A:
            to_remove = False
            for sym in line:
                if 'a' <= sym <= 'z' or 'A' <= sym <= 'Z':
                    to_remove = True
                    break

            if not to_remove:
                output_file.write(line + '\n')
