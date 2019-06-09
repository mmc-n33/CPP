clear all; close all; clc;

s=100; % mm
v=192; % mm/s
a=4999.97; % mm/s^2

tv=v/a; % time to reach v
sv=0.5*a*tv^2; % distance to reach v
sc=s-2*sv; % distance covered as constant v
tc=sc/v; % time as constant v
tt=tv*2+tc; % total time