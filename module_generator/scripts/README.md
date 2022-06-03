# Persistence

To use the service file to make your LKM persistent, do this on the command line:
1) Check if your Linux machine runs systemd

    - sudo systemctl --version
    If you see the version number on your screen, you are running systemd - if not, you can install systemd with your Linux package manager (apt / dpkg / rpm) - most Linuxes already use systemd.

2) If you have systemd installed, create a file in the directory /etc/systemd/system with the name that you want your service to be known by, for example
    sudo vi /etc/systemd/system/casper.service  
    https://github.com/CoolerVoid/casper-fs/tree/main/module_generator/scripts/casper.service

    And paste the contents of the service file that I sent, save that file and exit.

3) To enable the service you created in step 2, type (if your file is called casper.service):
     sudo systemctl enable casper

4) When you next reboot your machine, check the file /var/log/syslog to make sure your service was started successfully.

Tip by Paul Weston 

# Hide action of send keys of password

. I've attached a very simple script (manage.sh) which asks for the fake device name and password to (un)hide / (un)protect 
- This avoids the clear text passwords and fake device name appearing in the shell history.
- https://github.com/CoolerVoid/casper-fs/blob/main/module_generator/scripts/manage.sh

Tip by Paul Weston 
