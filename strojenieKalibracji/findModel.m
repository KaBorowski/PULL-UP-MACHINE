K=45;
T1=14;
T2=11;
Tp=0.1;

s = tf('s');
ciagly = tf(K,[T1*T2 T1+T2 1]);
dyskretny = c2d(ciagly, Tp, 'zoh');

load('data/0_255_9s.mat');
y=y/255;

figure;
hold on;
grid;
step(dyskretny, 0:Tp:45);
plot(y, 'r');
hold off;
