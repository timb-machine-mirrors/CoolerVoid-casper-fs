#!/usr/bin/python3
# Casper-fs v0.1'/'
#You nd yaml, pyyaml modules... 
from util import parser

rules_filename=""

# Get argvs of user's input
rules_filename = parser.arguments()

# load rules of firewall at directory rules
try:
    rules_casper=parser.Get_config(rules_filename)
except Exception as e:
    traceback.print_exc()
    print(" log error in config parser rules: "+str(e))
    exit(0)

# Load templates and generate
try:
    parser.start_generator(rules_casper)
except Exception as e:
    print(" log error in rule generator: "+str(e))
    exit(0)
