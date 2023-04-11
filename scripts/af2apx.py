#!/usr/bin/python3

import argparse
import re

# Initialize parser
parser = argparse.ArgumentParser()

# Adding optional argument
parser.add_argument("-i", "--Input", help = "Input file")

# Read arguments from command line
args = parser.parse_args()

in_file = open(str(args.Input), "r")
out_file = open(str(args.Input) + ".af", "w")

arg_count = 0
attack_count = 0
# dictionary of args. key is arg name, value is id.
args = {}
# dictionary of attacks
attacks = {}

for line in in_file:
    m = re.search(r"arg\((\w+)\)", line)
    if m:
        # arg_count += 1
        arg_name = str(m.group(1))
        if not arg_name in args:
            arg_count += 1
            args[arg_name] = arg_count
    m = re.search(r"att\((\w+),(\w+)\)", line)
    if m:
        arg1 = str(m.group(1))
        arg2 = str(m.group(2))
        if arg1 in args and arg2 in args:
            attacks.setdefault(args[arg1], set()).add(args[arg2])
            attack_count += 1
        else:
            print("One of the arguments did not appear before: " + arg1 + ":" + arg2)

in_file.close()


out_file.write("p af " + str(arg_count) + "\n")
for arg in attacks.keys():
    for enemy in attacks[arg]:
        out_file.write(str(arg) + " ")
        out_file.write(str(enemy))
        out_file.write("\n")

out_file.close()

print("Argument count:" + str(arg_count))
print("Arguments attacking another argument:" + str(len(attacks.keys())))
print("Attack count:" + str(attack_count))
