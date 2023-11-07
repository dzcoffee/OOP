# OOP_proj3

follow this : http://cau.ac.kr/~bongbong/oop23/
http://cau.ac.kr/~bongbong/oop23/oop_proj3.html
http://cau.ac.kr/~bongbong/oop23/project3.html

**What you need to do for this project**
Modify sample C++ code (given to you below) to make ARKANOID game and submit your code (compilable source file package, executable file) and demo video file of ARKANOID game through eclass.
follow the instructions, submission rulss, and requirements below.

**Instructions for MS-windows OS users**
1.Follow DirectX installation guide for successful compilation of the sample code below.
If opening VirtualLego.dsw file does not work, try opening VirtualLego.sln file.
In case of compilation failure related to DirectX, you may consider compile_error.jpg (this was written by a student in a previous OOP class.)
2. Download sample source code (sample executable code, example video of sample code execution)
Also, understand the logic (how the program works., how the classes are organized. what are member variables and member functions in each class. and so on) in the source code. You don't need to understand how to use DirectX functions.
3. compile it using Visual Studio (2013 or a later version). I tested the compilation and execution of above sample code with Visual Studio 2022 and it worked OK.
If it is not compiled, make sure that directory setting for DirectX is properly done (installation guide).
4. modify the source code to make ARKANOID game that looks like this: (picture , video).
Some of functions you have to implement :
bool CSphere::hasIntersected(CSphere& ball) { } // check if there is collision between two spheres
void CSphere::hitBy(CSphere& ball) { } // what needs to be done if there is collision between two spheres.
bool CWall::hasIntersected(CSphere& ball) { } // check if there is collision between a sphere and a wall
void CWall::hitBy(CSphere& ball) { } // what needs to be done if there is collision between a sphere and a wall
You should additionally modify the program logic and code to make your own ARKANOID game. Execution of your program should look like this: example video
Note that the function "Display" is repeatedly called when you execute the code. You have to use the variable 'timeDelta' (time difference between two consecutive frames) appropriately.
5. Execute and test it.

**Instructions for MacOS or LINUX OS users**
The instructions for MacOS or LINUX OS users are basically the same as the instructions for MS-windows users, but
the difference is that you may have to use a different sample code opengl_arkanoid.cpp to start with. This code uses OpenGL and GLUT for graphics processing, which may work on MacOS and LINUX. Note that DirectX is not working on MacOS or LINUX.
For the MacOS user's OpenGL and GLUT setting, you may want to watch the youtube video: XCODE (Mac)
I have not tested this code in LINUX but this code may be compilable and executable with appropriate code adjustment, installation and setting. You may follow the youtube video for the installation. In this case, you may have to use line 8 instead of line 6 in the sample code.

**Team #9 **
should develop 3D Billiard game (i.e. 4 balls or pool (pocketball) billiards) as a team project and give a presentation (English).. The teams should also show execution demo of their program. Team project report (English) as well as source code and presentation files should be also submitted to eClass.
Execution example of 3D billiard game: Good-billiard.mp4, pool.mp4
