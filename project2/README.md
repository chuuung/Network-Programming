# Network Programming Project 2 - Remote Working Ground (rwg) Server

## Introduction
1. Design a Concurrent connection-oriented server. This server allows one client connect to it.
2. Design a server of the chat-like systems, called remote working systems (rwg). In this system, users can communicate with other users. You need to use the single-process concurrent paradigm to design this server.

## Scenario of Part One
You can use telnet to connect to your server.  
Assume your server is running on nplinux1 and listening at port 7001.  
bash$ telnet nplinux1.cs.nctu.edu.tw 7001  
% ls | cat  
bin test.html  
% ls |1  
% cat  
bin test.html  
% exit  
bash$  

## Scenario of Part Two
1. Pipe between different users. Broadcast message whenever a user pipe is used. 
2. Broadcast message of login/logout information.
3. New built-in commands:
- who: show information of all users. 
- tell: send a message to another user. 
- yell: send a message to all users.
- name: change your name.
4. All commands in project 1

```
bash$ telnet nplinux1.nctu.edu.tw 7001
***************************************
** Welcome to the information server **
***************************************
*** User ’(no name)’ entered from 140.113.215.62:1201. ***  # Broadcast message of user login
% who
<ID>    <nickname>  <IP:port>           <indicate me>
1       (no name)   140.113.215.62:1201 <- me
% name Jamie
*** User from 140.113.215.62:1201 is named ’Jamie’. ***
% ls
bin test.html
% *** User ’(no name)’ entered from 140.113.215.63:1013. *** # User 2 logins
who
<ID>    <nickname>  <IP:port>           <indicate me>
1       Jamie       140.113.215.62:1201 <- me
2       (no name)   140.113.215.63:1013
% *** User from 140.113.215.63:1013 is named ’Roger’. ***
who
<ID>    <nickname>  <IP:port>           <indicate me>
1       Jamie       140.113.215.62:1201 <- me
2       Roger       140.113.215.63:1013
% *** User ’(no name)’ entered from 140.113.215.64:1302. *** # User 3 logins
who
<ID>    <nickname>  <IP:port>           <indicate me>
1 Jamie
2 Roger
3 (no name)
% yell Who knows how to make egg fried rice? help me plz!
*** Jamie yelled ***: Who knows how to make egg fried rice? help me plz!
% *** (no name) yelled ***: Sorry, I don’t know. :-( # User 3 yells
*** Roger yelled ***: HAIYAAAAAAA !!! # User 2 yells
% tell 2 Plz help me, my friends!
% *** Roger told you ***: Yeah! Let me show you the recipe #User2tellstoUser1
*** Roger (#2) just piped ’cat EggFriedRice.txt >1’ to Jamie (#1) *** # Broadcast message of user pipe *** Roger told you ***: You can use ’cat <2’ to show it!
cat <5
*** Error: user #5 does not exist yet. ***
% cat <2
*** Jamie (#1) just received from Roger (#2) by ’cat <2’ ***
Ask Uncle Gordon
Season with MSG !!
% tell 2 It’s works! Great!
% *** Roger (#2) just piped ’number EggFriedRice.txt >1’ to Jamie (#1) ***
*** Roger told you ***: You can receive by your program! Try ’number <2’!
number <2
*** Jamie (#1) just received from Roger (#2) by ’number <2’ ***
1 1 Ask Uncle Gorgon
2 2 Season with MSG !!
% tell 2 Cool! You’re genius! Thank you!
```

