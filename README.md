# 3D Scene From Primitives 
Sample app for practice building 3D scenes using OpenGL, mesh primitives, transformations, and rendering tools.

## Reference Image

![image](./Project%20target%20image.png)

## Final Scene

![image](./Screenshot%202024-12-18%20212013.png)

## Build Instructions

### Visual Studio 

 0. use the following commands to ensure you have Visual Studio with the correct packages installed.

 ```
 winget install Microsoft.VisualStudio.2022.BuildTools 
 ```
 ```
 "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\setup.exe" modify^
     --passive^
     --channelId VisualStudio.17.Release^
     --productId Microsoft.VisualStudio.Product.BuildTools^
     --add Microsoft.VisualStudio.Component.Vcpkg
 ```
 install your preferred workflow but be sure to include the **vcpkg package manager**.

 1. clone repository

 2. open the .sln file in this repo from file explorer or visual studio

 3. open the developer terminal (Tools>Command Line>Developer Command Prompt) and run the following command to install dependencies
 
 ```
 vcpkg install --triplet x64-windows
 ```
 if you want to build for a different system then you will need to reconfigure the project and pass a different triplet argument

  4. once all of this has been configured you should be able to use the play button in Visual Studio to build and run the program.

## Reflection (CS330 Coursework)

### How do I approach designing software?

This project has helped me get some practice working with existing projects. I think it's also good practice for thinking around different APIS. much of this project was using the premade code to implement different features of the final scene. not only was this like working with a custom api but it also involved several operations using GLM types.
For designing many of the elements of this scene I would think of what tasks I could seperate, and then try and work through them using different strategies. For example choosing to work on the lamp posts in the first week, the benches and fences in another, and the trees and other finishing touches more towards the end. I think that this methodology helps facilitate flexability in solutions while still adhering to the required project structure. Since I still have much room to grow with style it can lead to some inconsistancies in my code, but it's much better than feeling pigeonholed by those bad decisions later on. This is part of why working within an existing framework is useful while I lack experience.
In the future I hope to be able to build larger pieces of software with more moving parts. I think that my methodology can be expanded to such a scope by focusing on component modularity as even if my overall architecture decicions start to have issues from my lack of experience, having well constructed component parts means that it's more likely that I can reuse them with the new overal architecture.

### How do I approach developing programs?

Having my design focus on seperating the project into managable chunks menas that a lot of the development process is choosing one of the chunks to work on and then improving on it until it feels complete. In the best cases this could just be one block of time, but returing to old problems is always something that I can be itching to do when I learn more later on. This is where tasks like review or refacotring can be great opporotunities if the time exists for them of course.
My project used iteration heavily, not only in the macro design where many components were built off of the backs of previous ones, but also in the structure of the scene itself with components needing to be rendered multiple times meaning that solving how to draw the object at all could be one iteration followed by an iteration of how to render all the instances in the scene seperately. I think identifying the tasks that need to be completed and working through them one by one is a great starting point to iterate over as what is a worthwile task can change over the course of a projects development.
I think that this project didn't really have many moments where my development process changed as I think i did a good job at the start of determining which tasks would be easier and which would be harder. much of my ability to complete later tasks was due to not having to reconsider the solutions to previous problems over again, for example many of the objects used a similar algorithm to render many of the elements meaning i could focus on the complexity of the trees at the end instead of worrying about the quantity.

### How can computer science help me in reaching my goals? 

I think that computer science is most useful in how it can automate many processes. this can range from something as simple as copying a word 100 times or as complicated as developing predictive models for the weather or physical phenomenon. The trick is being able to see those problems and have the skill to actually come up with a way to solve them with software.
Computer graphics is one of the fields in computer science that is an easy on ramp to many of the more complex set of problems as the whole process is built off of reasonably complex mathematics and thats just drawing the pictrues without yet considering what is being drawn or why. Software like flight simulators or medical imaging tools use this as a baseline to solve increasingly complex problems.
I hope that in the future I can do more with computational graphcis because I believe that many of the most interesting problems benefit greatly from having new and novel wasy to think about them. I think that visualizations in general are also just a great way to interact with the people solving these problems which can help foster innovation indirectly.
