#!/bin/bash

while true
  do
    gpg --batch --out output --passphrase adminj310a -r 33E8625A -d "plaintext1.gpg" 
    sleep 0.01
  done
