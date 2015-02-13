#! /bin/bash

# generate ssh key
ssh-keygen -t rsa -C "your_email@youremail.com" 

cat ~/.ssh/id_rsa.pub

ssh -T git@github.com 