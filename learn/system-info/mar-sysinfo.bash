#!/usr/bin/env bash
set -euo pipefail

out=report
mkdir -p $out

# what CPU(s) is there, and what does it support?
lscpu >$out/cpu.txt
# what memory is there?
lsmem >$out/memory.txt
# what storage is there?
lsblk >$out/storage.txt
# what graphics card is there?
# what networking is there?
# what interface devices?
# graphics card
# mouse, keeb
# speakers, mic, camera
# what other peripherals? (usb, i2c, idk)
lsclocks >$out/hw-extra.txt
# what connections?
# network firewall setup, active connections+rates
# open file descriptors, ig
# users
lslogins >$out/users.txt
# what UI stack is running? (window manager, desktop environment, login manager, &c)
# what applications and tools are there?
# what logs are there and where? Same for configs
# how does the system search for libraries/headers?

# what versions of what software is running?
# what do we know about those software?
# scan for known security vulns
