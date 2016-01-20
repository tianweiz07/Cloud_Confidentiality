#!/bin/bash

while true
  do
    gpg --batch --output output --passphrase adminj310a -r 331A2EF2 -d "plaintext1.gpg" 
  done
