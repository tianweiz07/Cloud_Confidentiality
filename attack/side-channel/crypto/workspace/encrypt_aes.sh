#!/bin/bash

while true
  do
    openssl enc -aes-256-cbc -in plaintext1 -out output -pass pass:adminj310a
    sleep 0.01
  done
