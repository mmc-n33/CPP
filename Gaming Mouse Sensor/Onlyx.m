function [tn,dxn]=Onlyx(t,dx)

dxn=dx;
tn=t;
tn(dxn==0)=[];
dxn(dxn==0)=[];
tn=tn-tn(1);


end