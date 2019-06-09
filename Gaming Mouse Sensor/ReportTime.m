function [dt,tt]=ReportTime(t)

tt=zeros(length(t)-1,1); % time difference between two adjacent detections
for i=1:length(t)-1
    tt(i)=t(i+1)-t(i);
end

newtt=exclude3std(tt); % exclude 3 std
newtt(isnan(newtt))=[]; % get rid of NaN

dt=mean(newtt); % mean of the time difference that occurs dominantly

end