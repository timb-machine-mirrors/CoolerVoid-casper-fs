# Use this script to hidden the action to send keys to Casper-fs.
# Using this script, you cannot see the history command.
# save this script in /usr/bin/
# COntribution by Paul Wiston - contact: paul.w3st0n@googlemail.com
echo -n "Enter device: "
read DEVICE

echo -n "Enter key: " 
stty -echo
read KEY
stty echo

sudo su - root -c "echo $KEY > $DEVICE"
echo
