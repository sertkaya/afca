#!/usr/bin/python3
import argparse
import collections
import random
import sys


# Initialize parser
parser = argparse.ArgumentParser()

# Adding optional parameter
parser.add_argument('args_count', help="Number of arguments")
parser.add_argument('inner_probability', help="Probability of the attacks relation")
parser.add_argument('outer_probability', help="Probability of the attacks relation")
parser.add_argument('scc_count', help="Minimum number of strongly connected components")
parser.add_argument('out_file', help = "Output file")

# Read parameters from command line
params = parser.parse_args()

out_file_af = open(str(params.out_file) + ".af", "w")
out_file_apx = open(str(params.out_file) + ".apx", "w")
arg_count = int(params.args_count)
inner_probability = float(params.inner_probability)
outer_probability = float(params.outer_probability)
scc_count = int(params.scc_count)

if (inner_probability > 1 or outer_probability > 1):
    sys.exit("Probability cannot be larger than 1.")

component = []
while set(component) != set(range(scc_count)):
    component = [random.randrange(scc_count) for i in range(arg_count)]
    
outer_attacks = [[random.random() < 0.3 for d in range(scc_count)]
                 for c in range(scc_count)]
# dictionary of attacks
attacks = collections.defaultdict(set)
for i in range(arg_count):
    for j in range(arg_count):
        if i == j:
            continue
        if component[i] == component[j]:
            if random.random() < inner_probability:
                attacks[i + 1].add(j + 1)
        elif (component[i] < component[j] and
              outer_attacks[component[i]][component[j]] and
              random.random() < outer_probability):
            attacks[i + 1].add(j + 1)


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
