#!/usr/bin/python3

import argparse
import re
import sys
import numpy as np
from random import randrange

# Initialize parser
parser = argparse.ArgumentParser()

# Adding optional parameter
parser.add_argument('args_count', help = "Number of arguments")
parser.add_argument('density', help = "Density of the attacks relation")
parser.add_argument('out_file', help = "Output file")

# Read parameters from command line
params = parser.parse_args()

out_file_af = open(str(params.out_file) + ".af", "w")
out_file_apx = open(str(params.out_file) + ".apx", "w")
arg_count = int(params.args_count)
density = float(params.density)

if (density > 1):
    sys.exit("Density cannot be larger than 1.")

# dictionary of attacks
attacks = {}
attack_count = 0
while ((attack_count / (arg_count*arg_count)) <= density):
    row = randrange(1,arg_count)
    column = randrange(1,arg_count)
    if row not in attacks:
        attacks.setdefault(row, set()).add(column)
        attack_count += 1
    elif column in attacks[row]:
        continue
    else:
        attacks[row].add(column)
        attack_count += 1


# dictionary of args. key is arg name, value is id.
# args = {}

# 
# for line in in_file:
#     m = re.search(r"arg\((\w+)\)", line)
#     if m:
#         # arg_count += 1
#         arg_name = str(m.group(1))
#         if not arg_name in args:
#             arg_count += 1
#             args[arg_name] = arg_count
#     m = re.search(r"att\((\w+),(\w+)\)", line)
#     if m:
#         arg1 = str(m.group(1))
#         arg2 = str(m.group(2))
#         if arg1 in args and arg2 in args:
#             attacks.setdefault(args[arg1], set()).add(args[arg2])
#             attack_count += 1
#         else:
#             print("One of the arguments did not appear before: " + arg1 + ":" + arg2)
# 
# in_file.close()
# 
# 
out_file_af.write("p af " + str(arg_count) + "\n")
for arg in attacks.keys():
    for enemy in attacks[arg]:
        out_file_af.write(str(arg) + " ")
        out_file_af.write(str(enemy))
        out_file_af.write("\n")
out_file_af.close()

for arg in attacks.keys():
    out_file_apx.write("arg(" + str(arg) + ").\n")
for arg in attacks.keys():
    for enemy in attacks[arg]:
        out_file_apx.write("att(" + str(arg) + "," + str(enemy) + ").\n")
out_file_apx.close()
# 
print("Argument count:" + str(arg_count))
print("Arguments attacking another argument:" + str(len(attacks.keys())))
print("Attack count:" + str(attack_count))
