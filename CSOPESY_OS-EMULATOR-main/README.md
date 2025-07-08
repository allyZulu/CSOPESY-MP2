CSOPESY-OS EMULATOR PROJECT (Phase 1)


Developers:
- Alcantara, Keira S22
- Brodeth, Charlize S13
- Fernandez, Candice S22
- Zulueta, Alliyah S22

How to run:
1. Make sure c++ version is at least version 11 onwards
2. Make sure all files are in the same directory (including the config.txt file) 
3. Set up the config.txt file with your desired configurations
4. Open up the command line and make sure you are in the right directory
5. Compile using: g++ -o os_emulator.exe main.cpp ConsoleManager.cpp Scheduler.cpp Process.cpp Instruction.cpp
6. Run using : os_emulator.exe
7. After running the program type in “initialize” command to start the emulator, it will set up the emulator given the configurations found in the text file
8. Enter “scheduler-start” to start the scheduling algorithm and continuously produce dummy processes
9. To create user defined processes type “screen -s <process name>” and within it type “process-smi” to check details of that process
10. Type in “screen-ls” to show all of the processes and their status
11. Type in “report-util” to have a text file summary of all the processes
12. Type in “screen -r <process name>” to show details of the process, if it is finished or not
13. Type in “scheduler-stop” to stop the scheduling algorithm
14. Lastly, type in “exit” command to fully exit the program
