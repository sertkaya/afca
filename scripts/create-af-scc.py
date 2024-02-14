#!/usr/bin/python3
import argparse
import collections
import random
import sys


# Initialize parser
parser = argparse.ArgumentParser()

# Adding optional parameter
parser.add_argument('args_count', help="Number of arguments")
parser.add_argument('density', help="Density of the attacks relation")
parser.add_argument('scc_count', help="Minimum number of strongly connected components")
parser.add_argument('out_file', help = "Output file")

# Read parameters from command line
params = parser.parse_args()

out_file_af = open(str(params.out_file) + ".af", "w")
out_file_apx = open(str(params.out_file) + ".apx", "w")
arg_count = int(params.args_count)
density = float(params.density)
scc_count = int(params.scc_count)

if (density > 1):
    sys.exit("Density cannot be larger than 1.")

component = []
while set(component) != set(range(scc_count)):
    component = [random.randrange(scc_count) for i in range(arg_count)]
    
# dictionary of attacks
attacks = collections.defaultdict(set)
for i in range(arg_count):
    for j in range(arg_count):
        # if i == j:
        #     continue
        if random.random() < density:
            if component[i] <= component[j]:
                attacks[i + 1].add(j + 1)
            else:
                attacks[j + 1].add(i + 1)
                

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
print(f"Attack count: {sum(len(a) for a in attacks.values())}")
