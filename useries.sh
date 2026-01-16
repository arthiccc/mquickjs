#!/bin/bash

show_menu() {
    clear
    echo -e "\e[1;36m========================================\e[0m"
    echo -e "\e[1;36m       THE µSERIES BASH SELECTOR\e[0m"
    echo -e "\e[1;36m========================================\e[0m"
    echo ""
    echo -e " 1. \e[1;33mµGauntlet Pro\e[0m (16KB 3D Engine)"
    echo -e " 2. \e[1;33mµStatic\e[0m      (Minimalist SSG)"
    echo -e " 3. \e[1;33mµBrain\e[0m       (Brainfuck JIT)"
    echo -e " 4. \e[1;33mµCHIP-8\e[0m      (12KB Emulator)"
    echo -e " 5. \e[1;33mµGauntlet\e[0m    (Original Benchmarks)"
    echo -e " 6. \e[1;32mRun Multi-Tool (C version)\e[0m"
    echo -e " q. Exit"
    echo ""
    read -p "Select an option: " choice
}

while true; do
    show_menu
    case $choice in
        1)
            echo "Starting µGauntlet Pro..."
            ./gauntlet_pro world.js
            ;;
        2)
            echo "Starting µStatic..."
            ./ustatic ssg.js
            ;;
        3)
            echo "Starting µBrain..."
            ./ubrain brain.js
            ;;
        4)
            echo "Starting µCHIP-8..."
            ./uchip8 chip8.js test.ch8
            ;;
        5)
            echo "Starting µGauntlet..."
            ./gauntlet benchmarks.js 32
            ;;
        6)
            ./useries
            ;;
        q)
            echo "Goodbye!"
            exit 0
            ;;
        *)
            echo -e "\e[1;31mInvalid option\e[0m"
            sleep 1
            ;;
    esac
    echo ""
    read -p "Press Enter to return to menu..."
done
