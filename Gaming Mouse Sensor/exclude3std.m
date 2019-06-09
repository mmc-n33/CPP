function [p,mew,var,Ngood]=exclude3std(p)

N=length(p);
mew=mean(p);
sig=std(p);
    
for i=1:3
    sx=0;
    sxx=0;
    Ngood=0;
    for j=1:N
        if isnan(p(j))
        else if abs((p(j)-mew)) > 3*sig
                p(j)=[];
            else
                p(j)=p(j);
                sx=sx+p(j);
                sxx=sxx+p(j)*p(j);
                Ngood=Ngood+1;
            end
        end
    end
    mew=sx/Ngood;
    sig=sqrt(sxx/Ngood-(sx/Ngood)^2);
end

var=sig^2;

end
