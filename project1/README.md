## Scenario of using npshell
A. Some important settings
1. The structure of the working directory
working_dir 
├─ b​in         
│ ├─​cat
│ ├─​ls
│ ├─​noop   # A program that does nothing
│ ├─ ​number    # Add a number to each line of input
│ ├─ ​removetag # Remove HTML tags and output to STDOUT
│ └─ ​removetag0 # Same as removetag, but outputs error
└─ test.html # messages to STDERR.

  
2. In addition to the above executables, the following are built-in commands
supported by your npshell
a) setenv b) printenv c) exit