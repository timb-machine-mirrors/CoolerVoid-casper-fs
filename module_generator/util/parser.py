import os
import yaml
import argparse
from util import file_op
from termcolor import colored

def banner():
    print("\t____ ____ ____ ___  ____ ____    ____ ____") 
    print("\t|    |__| [__  |__] |___ |__/ __ |___ [__  ") 
    print("\t|___ |  | ___] |    |___ |  \    |    ___] ")
    file = open("util/banner.txt", "r")
    logo = file.read()
    print(colored(logo,'white',attrs=['blink']) )
    file.close()     
    print(colored("\tCustom Linux Kernel Module generator to turn private files hidden. The second function is to protect confidential files to prevent reading, writing and removal.",'yellow'))
    print(colored("\tVersion 0.1 coded by CoolerVoid",'cyan'))
    print ("\n---\nExample to use:\n")
    print ("\tpython3 Casper-fs.py --rules rules/my_secret_files.yaml\n")
    print ("\n github.com/CoolerVoid/casper-fs\n....\n")

def arguments():
    parser = argparse.ArgumentParser(description = banner())
    parser.add_argument('-r', '--rules', action = 'store', dest = 'rules_filename', default="rules/fs-rules.yaml", required = True, help = 'file system rules to create a kernel module, look directory rules')
    args = parser.parse_args()
    if os.path.isfile(args.rules_filename):
        return args.rules_filename
    else:
        print ('File rules does not exist!')
        exit(0)
    parser.print_help()
    exit(0)

def Get_config(ConfigFile):
 d={}
 d['hide_list_fs']=""
 d['protect_list_fs']=""
 document = open(ConfigFile, 'r')
 parsed = yaml.load(document, Loader=yaml.FullLoader)
 print("Load external config "+ConfigFile)

 for key,value in parsed.items():
  if key == "binary_name":
   d['binary_name']=str(value)
  if key == "module_name":
   d['module_name']=str(value)
  if key == "unhide_module_key":
   d['unhide_module_key']=str(value)
  if key == "hide_module_key":
   d['hide_module_key']=str(value)
  if key == "fake_device_name":
   d['fake_device_name']=str(value)
  if key == "unhide-hide-file-key":
   d['unhide-hide-file-key']=value
  if key == "unprotect-protect-file-key":
   d['unprotect-protect-file-key']=value
  if key == "fs-rules":
   e=value
   print(e)

 exit(0)

 for v in e:
     for value in v['hidden']:   
         d['hide_list_fs']=d['hide_list_fs']+value+","

 for v in e:
     for value in v['protect']:   
         d['protect_list_fs']=d['protect_list_fs']+value+","
  
 d['hide_list_fs']=d['hide_list_fs'][:-1]
 d['protect_list_fs']=d['protect_list_fs'][:-1]

 return d




def start_generator(rules_fs):
 v = rules_fs
 # Load content of templates
 template_content_main=file_op.File_to_string("template/main.c")
 template_content_hooked=file_op.File_to_string("template/hooked.c")
 template_content_hooked_h=file_op.File_to_string("template/hooked.h")
 template_content_makefile=file_op.File_to_string("template/Makefile")
 print ("\n Generate Kernel module \n")
 # main.c
 main_file=template_content_main
 main_file=main_file.replace("CASPER_MODULE",v['module_name'])
 main_file=main_file.replace("CASPER_FAKE_DEVICE",v['fake_device_name'])
 # hooked.c
 hooked_file=template_content_hooked
 hooked_file=hooked_file.replace("CASPER_HIDE",v['hide_module_key'])
 hooked_file=hooked_file.replace("CASPER_UNHIDE",v['unhide_module_key'])
 hooked_file=hooked_file.replace("CASPER_HIDE_UNHIDE_FS",v['unhide-hide-file-key'])
 hooked_file=hooked_file.replace("CASPER_PROTECT_UNPROTECT_FS",v['unprotect-protect-file-key'])
 hooked_file=hooked_file.replace("CASPER_PROTECT_LIST",v['protect_list_fs'])
 hooked_file=hooked_file.replace("CASPER_HIDE_LIST",v['hide_list_fs'])
 # hooked.h
 hooked_h_file=template_content_hooked_h
 hooked_h_file=hooked_h_file.replace("CASPER_MODULE_NAME",v['module_name'])
 # Makefile
 makefile_file=template_content_makefile
 makefile_file=makefile_file.replace("CASPER_BINARY_NAME",v['binary_name'])
# save output
 output_makefile="output/Makefile"
 output_hooked="output/hooked.c"
 output_hooked_h="output/hooked.h"
 output_main="output/main.c"
 file_op.WriteFile(output_makefile,makefile_file)
 file_op.WriteFile(output_hooked,hooked_file)
 file_op.WriteFile(output_hooked_h,hooked_h_file)
 file_op.WriteFile(output_main,main_file)
 print ("Python Script end, please look the output your custom kernel module at directory output/\n")



