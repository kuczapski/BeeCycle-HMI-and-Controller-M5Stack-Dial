The main purpose of the application to allow a simple interface to control the rotation speed and programs of a honey centrifuge.



The application will run on an M5Stack Dial rotary display and will be structured as such:



1\. PWM Control core. 

This core controls two PWM output of the microcontroller. 

It has two state variables:

&#x20; a.) currentAppliedDutyCycle - It is the value currently set.

&#x20;     Positive means that PWM output 1 is use, and negative means that PWM output 2 is used. 

&#x20;     Thus the sign indicates the rotation direction of a DC motor controlled by a H bridge



&#x20; b.) targetRotationSpeedPercentage - This is where the current program or human interaction tells how fast the motors should spin. 0% - stopped, 100% maximum allowed duty cycle based on the configuration



And two configuration parameters

&#x20; a.) zereToMaxSpinupTime - Tells how fast should the currentAppliedDutyCycle increase from 0% to 100% and also the other    direction 100% to 0%, This is to limit high acceleration or deceleration of the motor

&#x20;  

&#x20; b.) maxDutyCycle - The maximum allowed duty cycle . 



There is a continuously running logic updating the currentAppliedDutyCycle based on the targetRotationSpeedPercentage and configuration params.

The logic first calculate the targetDutyCycle=targetRotationSpeedPercentage\*maxDutyCycle

Than if the targetDutyCycle differs from the currentAppliedDutyCycle than the currentAppliedDutyCycle is updated considering the zereToMaxSpinupTime parameter and the time spend since the last update. 

The formula should ensure that there is no sudden acceleration on the motor and that there is a soft start and stop. 

Also, the formula should take into consideration that nonlinear nature of the kinetic energy stored in the spinning object and increase or decrease the dutyCycle in a way which make the kinetic energy change constant. 





2\. User Interface Layer.

The user interface is structured in pages, with a "header-half-ring" always visible.



2.1 The header-half-ring indicates the current state of the targetRotationSpeedPercentage and currentAppliedDutyCycle

It occupies the upper half arc of the circular display and:

&#x20;  a. has a 4px wide ORANGE arc showing the current applied rotation speed going to the left or right of depending on the rotation direction

&#x20;  b. has a small 6px diameter circle showing where is the target rotation speed.





2.2. UI Pages. 

There will be 3 application modes or pages. The user can change the page using the rotary input of the display or enter/activate the page using a short click on the display.

The user can exit the page using a long click.



2.2.1 Standard Centrifuge program. 

\- When being in this mode one click on the display will start centrifuge cycle which:

&#x20;     1.) Spins up the motor to 25% of the max speed to the left than stops it

&#x20;     2.) Spins up the motor to 50% of the max speed to the right than stops it

&#x20;     3.) Spins up the motor to 75% of the max speed to the left than stops it

&#x20;     4.) Spins up the motor to 100% of the max speed to the right than stops it

&#x20;- The program can be stopped at any time with a short click 

&#x20;- In the middle of the screen a total progress is indicated

&#x20;- While the program is stopped. The user can use the rotary input to increase or decrease the duration of the whole program. 

&#x20;  (minimum duration is limited by the zereToMaxSpinupTime paramter





2.2.2. Manual Centrifuge program 

\- When being in this mode, by rotating the rotary input the user can select the desired max rotation speed and direction

\- When the button is clicked the target speed is set and the motor starts to spin up 

\- During spinning the target rotation speed can be continuously change using the rotary imput

\- When the button is clicked again the motor is stopped. 

&#x20;

2.2.3. Parameter Editor mode

Using this mode the parameters can be edited. 

After entering the mode a parameter name will be shown in the upper prat of the scree and in the middle the set value.

Value will be changed using the rotary input and the displayed parameter can be changed by short click.

Long click will exit the mode and store the updated parameters in the flash of the microcontroller





Notes:

\- Display rendering must be done with double buffering to ensure fluent animation

\- Page changes must be done with sliding views

\- Parameters must be stored permanently in a non volatile memory of the controller

&#x20;     













