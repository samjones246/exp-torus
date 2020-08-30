#!/bin/bash

if false; then
  adb shell '
    while true; do
        for c in $(seq 1 256); do
            input tap 570 1760;
            sleep 2;
            input tap 170 840;
            sleep 2;
        done;
        input tap 200 1920;
        sleep 2;
        input tap 550 888;
        sleep 2;
        input tap 280 1240;
        sleep 2;
        
        input tap 170 940;
        sleep 2;
        input tap 170 840;
        sleep 2;
        input tap 170 940;
        sleep 2;
    done
  '
fi

  adb shell '
    while true; do
        for c in $(seq 1 256); do
            input tap 555 444;
            sleep 2;
        done;
        input tap 200 1920;
        sleep 2;
        input tap 550 888;
        sleep 2;
        input tap 280 1240;
        sleep 2;
    done
  '
