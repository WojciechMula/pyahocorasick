#!/bin/sh
aspell dump master en | aspell expand | tr " " "\n" > bug_56_words.txt
