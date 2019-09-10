clear all
clc
 
arduino=serial('/dev/cu.usbmodem14101','BaudRate',9600);
arduino.timeout = 100;
 
fopen(arduino);
 
x=linspace(1,100);
y=linspace(1,100);

   
for i=1:length(x)
	y(i)=fscanf(arduino,'%d');
    plot(x,y);
    drawnow;
end
	
fclose(arduino);
disp('making plot..')
plot(x,y);