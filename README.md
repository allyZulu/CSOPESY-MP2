
Final CSOPESY MO2 is in the **mo2 Branch**

Developers:
Alcantara, Keira S22
Brodeth, Charlize S13
Fernandez, Candice S22
Zulueta, Alliyah S22
How to run:

1. Make sure c++ version is at least version 11 onwards
2. Make sure all files are in the same directory (including the config.txt file)
3. Set up the config.txt file with your desired configurations
4. Open up the command line and make sure you are in the right directory
5. Compile using: g++ -o os_emulator.exe main.cpp ConsoleManager.cpp Scheduler.cpp Process.cpp Instruction.cpp MemoryManager.cpp
6. Run using : os_emulator.exe
7. After running the program type in “initialize” command to start the emulator, it will set up the emulator given the configurations found in the text file
8. Enter “scheduler-start” to start the scheduling algorithm and continuously produce dummy processes
9. To create user defined processes type “screen -s [process name] [process memsize] ” and within it type “process-smi” to check details of that process
10. To create user defined processes with instructions type “screen -c [process name] [process memsize] [instructions]” 
11. Type in “screen-ls” to show all of the processes and their status
12. Type in “report-util” to have a text file summary of all the processes
13. Type in “screen -r ” to show details of the process, if it is finished or not
14. Type in "vmstat" to show details of the memory and access backing store details
15. Type in "process-smi" on the main menu to see the frame allocation
16. Type in “scheduler-stop” to stop the scheduling algorithm
17. Lastly, type in “exit” command to fully exit the program
