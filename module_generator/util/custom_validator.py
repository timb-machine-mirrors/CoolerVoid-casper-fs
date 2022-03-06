from schema import Schema, Optional, SchemaError, Regex
import yaml
import os
from termcolor import colored

config_schema = Schema({

        "binary_name": Regex(r'^[a-zA-Z0-9]{3,64}$'),
        "module_name": Regex(r'^[a-zA-Z0-9_]{3,64}$'),
        "unhide_module_key": Regex(r'^[a-zA-Z0-9]{3,64}$'),
        "hide_module_key": Regex(r'^[a-zA-Z0-9]{3,64}$'),
        "fake_device_name": Regex(r'^[a-zA-Z0-9]{3,64}$'),
        "unhide-hide-file-key": Regex(r'^[a-zA-Z0-9]{3,64}$'),
        "unprotect-protect-file-key": Regex(r'^[a-zA-Z0-9]{3,64}$'),
        "fs-rules": [
            {"hidden": { int : Regex(r'^[a-zA-Z0-9\._\-]{3,48}') }},
            {"protect": { int : Regex(r'^[a-zA-Z0-9\._\-]{3,48}') }}
        ]
}) #,ignore_extra_keys=True)

def is_valid_casper_yaml(input):
    configuration = input
    try:
        config_schema.validate(configuration)
    except SchemaError as se:
        for error in se.errors:
            if error:
                print(colored(error,"red"))
        for error in se.autos:
            if error:
                print(colored(error,"red"))
        #raise se
        exit(0)


