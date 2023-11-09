# Rootkit
- Utilizing C++ for kernel programming to conceal suspicious processes from system commands.
- In this project, I have implemented C functions that mimic the behavior of a Rootkit. The main goal is to hide suspicious processes in a way that users cannot detect any unusual activity when using system commands like `ls`, `ps`, `cat`, `cd`, `lsmod`, and `find`.
- [Detailed description](https://github.com/CaoRui0910/Rootkit/blob/main/Rootkit.pdf)

### How to implement Rootkit?
To hide the suspicious program, the process involves initially loading the original kernel settings, then loading the sneaky kernel settings to prevent my process from being discovered by users. After the completion of my process, the original kernel settings are restored, ensuring that users remain unaware of any changes or activities.

### Run
- `sudo ./sneaky_process`


### Test
- Use "Sneaky Kernel Module (a Linux Kernel Module – LKM)" part in [this file](https://github.com/CaoRui0910/Rootkit/blob/main/Rootkit.pdf)

### Note
- `Rootkit.pdf` includes details about this project.
